#pragma once

#include<string>
#include<dirent.h>
#include <Arduino.h>

class RecordNewFile
{
 private:
    /* data */
    char buf[13];
    int16_t cc=0;
    DIR *d;
    struct dirent *dir;
 public:
    char sdarr[15]="/sdcard/";
    RecordNewFile(/* args */);
   ~RecordNewFile();
    void writeFile();
};






