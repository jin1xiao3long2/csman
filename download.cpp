#include "download.hpp"
#include <stdio.h>
namespace {

    using testJ::download_task;

    size_t callback_file_length(void *data, size_t size, size_t nmemb, void *userp)
    {
        int r;
        long len = 0;

        r = sscanf(reinterpret_cast<char *>(data), "Content-Length: %ld\n", &len);
        if(r)
            *(reinterpret_cast<long *>(userp)) = len;
        return size * nmemb;
    }

    size_t callback_write(void *buffer, size_t size, size_t nmemb, void *userp)
    {
        auto *p = reinterpret_cast<FILE *>(userp);
        size_t wrote = fwrite(buffer, size, nmemb, p);
        return wrote;
    }

    size_t callback_progress(void *userp, double total, double wrote, double utotal, double uwrote)
    {
        auto *p = reinterpret_cast<download_task *>(userp);
        if(wrote == 0)
            return 0;
        printf("\rThe process bar: %ld / %ld : %d%%", static_cast<long>(wrote), static_cast<long>(total),
                static_cast<int>((wrote / total) * 100));
        p->can_resume(wrote);   //store data range in task
        return 0;
    }

}

void download_task::perform()
{
    if(_url.empty())
    {
        std::cout << "useless url" << std::endl;
        exit(-1); //change when changed to thread
    }

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(!curl)
    {
        std::cout << "initializing failed" << std::endl;
        exit(-1);
    }

    //get data
    curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _timeout);    //no http certificate
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    //resume
    long file_length = 0;
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &file_length);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, callback_file_length);

    //write
    FILE *fp;
    if((fp = fopen(this->_path.c_str(), "r")) != NULL)
    {
        char Oper;
        bool choose = false;
        do{
                if(!choose)
                        std::cout << _path << "already exists, overwrite it?(y/n)" << std::endl;
                else
                        std::cout << "Please enter 'Y' or 'N' " << std::endl;
        //some operations
        Oper = getchar();
        Oper = tolower(Oper);
        while(std::cin.get() != '\n');
        }while(Oper != 'y' && Oper != 'n');
        if(Oper == 'n'){
                std::cout << "download interrupt" << std::endl;
                exit(-1); //or orther choose        
        }
        fclose(fp);
    }
    fp = fopen(this->_path.c_str(), "wb");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<FILE *>(fp));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_write);


    //progress
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, reinterpret_cast<download_task *>(this));
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback_progress);

    if((res = curl_easy_perform(curl)) != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        std::cout << "\ndownload failed" << std::endl;
        exit(-1);
    }
    else
    {
        curl_easy_cleanup(curl);
        std::cout << "\ndownload success" << std::endl;
        return ;
    }
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("please enter:  %s  url file_path", argv[0]);
        return -1;
    }

    download_task *p = new download_task();
    p->set_url(argv[1]);
    p->set_path(argv[2]);
    p->perform();

    return 0;
}
