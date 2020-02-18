#pragma once

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <fstream>

namespace testJ{
    class download_task final{
    private:
        std::string _url;
        bool _resume = false;
        long _timeout = 30;
        long _start_pos = 0;
        FILE *_file = nullptr;
        std::string _path;

    public:
        download_task(std::string url, std::string path)
        {
            _url = url;
            _path = path;
        }

        download_task() = default;

        ~download_task() = default;

        download_task(const download_task &) = delete;

    public:
        void set_url(const std::string &url)
        {
            this->_url = url;
        }
        void can_resume(long position)
        {
            this->_resume = true;
            _start_pos = position;
        }
        void set_timeout(long timeout)
        {
            this->_timeout = timeout;
        }
        void set_path(std::string path)
        {
            this->_path = path;
        }

        void perform();
    };
}
       
