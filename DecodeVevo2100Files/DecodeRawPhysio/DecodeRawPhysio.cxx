/*=========================================================================
 *
 *  Copyright Kitware Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

// This header file <filename>CLP.h is auto-generated by cmake and contains
// the PARSE_ARGS macro that sets up command-line usage help and also parse
// command-line arguments
#include "DecodeRawPhysioCLP.h"

typedef struct
{
  unsigned int dwVersion;
  unsigned int dwNumFrames;
  unsigned int dwInfo;
  unsigned int dwReserved[7];

} VSI_RAW_FILE_HEADER;

typedef struct
{
  unsigned int dwTimeStamp;
  double       dbTimeStamp;
  unsigned int dwFrameNumber;
  unsigned int dwInfo;
  unsigned int dwPacketSize;
  unsigned int dwReserved[8];
  
} VSI_RAW_FRAME_HEADER; 

using namespace std;

void mfread(void * ptr, size_t size, size_t count, FILE * stream)
{
  size_t res = fread(ptr, size, count, stream);
  if(res != count) {
    cerr << "fread error" << endl;
    exit(2);
  }
}

int main(int argc, char *argv[])
{
  // sets up help for command-line usage and parses command-line arguments
  PARSE_ARGS;

  FILE *infile = fopen(in_raw_file.c_str(), "rb");

  if(infile == NULL) {
    cerr << "Error reading input raw file" << endl;
    exit(1);
  }

  FILE *outfile = fopen(out_csv_file.c_str(), "w");

  // read file header
  VSI_RAW_FILE_HEADER hfile;

  mfread(&hfile.dwVersion, sizeof(hfile.dwVersion), 1, infile);
  mfread(&hfile.dwNumFrames, sizeof(hfile.dwNumFrames), 1, infile);
  mfread(&hfile.dwInfo, sizeof(hfile.dwInfo), 1, infile);
  mfread(&hfile.dwReserved, sizeof(hfile.dwReserved), 1, infile);

  cout << "Version = " << hfile.dwVersion << endl;
  cout << "Num Frames = " << hfile.dwNumFrames << endl;
  cout << "Frame type = " << (hfile.dwInfo & 0x00000006) << endl;

  // read frames
  for(unsigned int i = 0; i < hfile.dwNumFrames; i++) {

    cout << "Reading frame " << i << "/" << hfile.dwNumFrames << endl;

    // read frame headers
    VSI_RAW_FRAME_HEADER hframe;

    mfread(&hframe.dwTimeStamp, sizeof(hframe.dwTimeStamp), 1, infile);
    mfread(&hframe.dbTimeStamp, sizeof(hframe.dbTimeStamp), 1, infile);
    mfread(&hframe.dwFrameNumber, sizeof(hframe.dwFrameNumber), 1, infile);
    mfread(&hframe.dwInfo, sizeof(hframe.dwInfo), 1, infile);
    mfread(&hframe.dwPacketSize, sizeof(hframe.dwPacketSize), 1, infile);
    mfread(&hframe.dwReserved, sizeof(hframe.dwReserved), 1, infile);

    bool isFrameInvalid = (hframe.dwInfo & 0x00000001);

    cout << "\tTime stamp = " << hframe.dbTimeStamp << endl;
    cout << "\tFrame number = " << hframe.dwFrameNumber << endl;
    cout << "\tFrame data size = " << hframe.dwPacketSize << endl;
    cout << "\tFrame invalid = " << isFrameInvalid << endl;

    // read frame data
    int nSamples = hframe.dwPacketSize / 8;

    cout << "\tNum samples = " << nSamples << endl;

    short *ecg = new short[nSamples];
    short *respiration = new short[nSamples];
    short *temperature = new short[nSamples];
    short *bp = new short[nSamples];

    mfread(ecg, sizeof(short), nSamples, infile);
    mfread(respiration, sizeof(short), nSamples, infile);
    mfread(temperature, sizeof(short), nSamples, infile);
    mfread(bp, sizeof(short), nSamples, infile);

    for(unsigned int j = 0; j < nSamples; j++) {

      if(i == 0 && j == 0) {
        fprintf(outfile, 
                "Frame, Timestamp, isFrameInvalid, Sample, ECG, Respiration, "
                "Temperature, Blood pressure");
      }

      fprintf(outfile, "\n%d, %f, %d, %d, %d, %d, %d, %d", 
              hframe.dwFrameNumber, hframe.dbTimeStamp, isFrameInvalid, j, 
              ecg[j], respiration[j], temperature[j], bp[j]); 
    }

    delete(ecg);
    delete(respiration);
    delete(temperature);
    delete(bp);
  }

  fclose(outfile);
  fclose(infile);

  return 0;
}