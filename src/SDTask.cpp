/*
 * SDTask.cpp
 *
 *  Created on: 28.07.2023
 *      Author: tobeh
 */




#include <cstdlib>

#include "FreeRTOS.h"
#include "TaskBase.h"
#include "../template/stm32facade.h"
#include "SDTask.h"
#include "diskio.h"
#include "ff.h"


using namespace Platform::BSP;

  //some variables for FatFs
  FATFS FatFs; 	//Fatfs handle
  FIL fil; 		//File handle
  FRESULT fres; //Result after operations

SDTask::SDTask(const char * const name,
            uint16_t    stackDepth,
			osPriority priority,
            bool        immidiateStart):Task(name, stackDepth, priority, immidiateStart){
}

void SDTask::write_to_file(const char* filename, const char* data)
{
	printf("SD-Write-Data \r\n");
    if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
    	UINT bytesWritten; //Todo: gucken ob uint16_t verwendbar
        f_write(&file, data, strlen(data), &bytesWritten);
        f_close(&file);
    }
}

void SDTask::read_from_file(const char* filename, char* buffer, UINT bufferSize)
{
	printf("SD-Read-Data \r\n");
    if (f_open(&file, filename, FA_READ) == FR_OK)
    {
    	UINT bytesRead; //Todo: gucken ob uint16_t verwendbar
        f_read(&file, buffer, bufferSize - 1, &bytesRead);
        buffer[bytesRead] = '\0'; // Nullterminierung, um den String korrekt zu beenden.
        f_close(&file);
    }
}

void SDTask::m_task(){
  printf("\r\n~ SD card demo by kiwih ~\r\n\r\n");

  HAL_Delay(1000); //a short delay is important to let the SD card settle


  //Open the file system
  fres = f_mount(&FatFs, "/", 1); //1=mount now
  if (fres != FR_OK) {
	printf("f_mount error (%i)\r\n", fres);
	while(1);
  }

  //Let's get some statistics from the SD card
  DWORD free_clusters, free_sectors, total_sectors;

  FATFS* getFreeFs;

  fres = f_getfree("", &free_clusters, &getFreeFs);
  if (fres != FR_OK) {
	printf("f_getfree error (%i)\r\n", fres);
	while(1);
  }

  //Formula comes from ChaN's documentation
  total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
  free_sectors = free_clusters * getFreeFs->csize;

  printf("SD card stats:\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", total_sectors / 2, free_sectors / 2);

  //Now let's try to open file "test.txt"
  fres = f_open(&fil, "test.txt", FA_READ);
  if (fres != FR_OK) {
	printf("f_open error (%i)\r\n", fres);
	while(1);
  }
  printf("I was able to open 'test.txt' for reading!\r\n");

  //Read 30 bytes from "test.txt" on the SD card
  BYTE readBuf[30];

  //We can either use f_read OR f_gets to get data out of files
  //f_gets is a wrapper on f_read that does some string formatting for us
  TCHAR* rres = f_gets((TCHAR*)readBuf, 30, &fil);
  if(rres != 0) {
	printf("Read string from 'test.txt' contents: %s\r\n", readBuf);
  } else {
	printf("f_gets error (%i)\r\n", fres);
  }

  //Be a tidy kiwi - don't forget to close your file!
  f_close(&fil);

  //Now let's try and write a file "write.txt"
  fres = f_open(&fil, "write.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
  if(fres == FR_OK) {
	printf("I was able to open 'write.txt' for writing\r\n");
  } else {
	printf("f_open error (%i)\r\n", fres);
  }

  //Copy in a string
  strncpy((char*)readBuf, "a new file is made!", 19);
  UINT bytesWrote;
  fres = f_write(&fil, readBuf, 19, &bytesWrote);
  if(fres == FR_OK) {
	printf("Wrote %i bytes to 'write.txt'!\r\n", bytesWrote);
  } else {
	printf("f_write error (%i)\r\n", fres);
  }

  //Be a tidy kiwi - don't forget to close your file!
  f_close(&fil);

  //We're done, so de-mount the drive
  f_mount(NULL, "", 0);
	while(1){

	}
}
