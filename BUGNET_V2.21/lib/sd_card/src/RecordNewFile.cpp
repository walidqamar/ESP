#include "RecordNewFile.h"
#include <esp32-hal-log.h>

RecordNewFile::RecordNewFile(/* args */)
{
}

void RecordNewFile::writeFile()
{
  d = opendir("/sdcard/");
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {   
      cc=cc+1;
      printf("%s\n", dir->d_name);
    }
    closedir(d);
  }
  else
  {
    printf("ERORR");
  }
  snprintf(buf, 12, "test%d.wav", cc); // puts string into buffer
  strcat(sdarr,buf);
}

RecordNewFile::~RecordNewFile()
{

}