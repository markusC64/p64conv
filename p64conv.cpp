
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "p64.h"

}

#include <string>
#include <iostream>
#include <fstream>
#include <re2/re2.h>

using namespace std;

int txt2p64(string infile, string outfile)
{
   TP64MemoryStream P64MemoryStreamInstance;
   TP64Image        P64Image;
   FILE *           pFile;
   uint32_t         lSize;
   uint8_t *        buffer;

   P64MemoryStreamCreate(&P64MemoryStreamInstance);
   P64ImageCreate(&P64Image);

   fstream in(infile.c_str());
   string  line;
   int     track = -1;
   double  number;
   RE2     trackPattern(" *track ([0-9]+(?:\\.5)?)");
   RE2     fluxPattern(" *flux ([0-9]+(?:\\.[0-9]+)?)");

   while (getline(in, line))
   {
      if (RE2::FullMatch(line, fluxPattern, &number))
      {
         int position = int(number + 0.5);

         if (position == 0)
            position = 1;

         P64PulseStreamAddPulse(&P64Image.PulseStreams[track], position, 0xffffffff);
      }
      else if (RE2::FullMatch(line, trackPattern, &number))
      {
         track = int(2 * number + 0.1);
      }
   }

   in.close();

   if (P64ImageWriteToStream(&P64Image, &P64MemoryStreamInstance))
   {
      printf("Write ok!\n");

      pFile = fopen(outfile.c_str(), "wb");

      fwrite(P64MemoryStreamInstance.Data, 1, P64MemoryStreamInstance.Size, pFile);
      fclose(pFile);
   }
   else
   {
      printf("Write failed!\n");
   }
}

int p642txt(string infile, string outfile)
{
   char             buffer2[1000];
   TP64MemoryStream P64MemoryStreamInstance;
   TP64Image        P64Image;
   FILE *           pFile;
   uint32_t         lSize;
   uint8_t *        buffer;

   P64MemoryStreamCreate(&P64MemoryStreamInstance);
   P64ImageCreate(&P64Image);

   pFile = fopen(infile.c_str(), "rb");

   if (pFile)
   {
      fseek(pFile, 0, SEEK_END);

      lSize = ftell(pFile);

      rewind(pFile);

      buffer = (uint8_t *) malloc(sizeof(uint8_t) * lSize);

      fread(buffer, 1, lSize, pFile);
      fclose(pFile);
      P64MemoryStreamWrite(&P64MemoryStreamInstance, buffer, lSize);
      P64MemoryStreamSeek(&P64MemoryStreamInstance, 0);

      if (P64ImageReadFromStream(&P64Image, &P64MemoryStreamInstance))
      {
         printf("Read ok!\n");

         ofstream out(outfile.c_str());

         for (int track = 2; track <= 85; track++)
         {
            TP64PulseStream & instance = P64Image.PulseStreams[track];
            int               current  = instance.UsedFirst;

            if (current >= 0)
            {
               sprintf(buffer2, "track %i%s\n", track / 2, track & 1 ? ".5" : "");

               out << buffer2;
            }

            while (current >= 0)
            {
               int          pos = instance.Pulses[current].Position;
               unsigned int str = instance.Pulses[current].Strength;

               if (str == 0xffffffff)
                  sprintf(buffer2, "   flux %i\n", pos);
               else
                  sprintf(buffer2, "   flux %i %i\n", pos, str);

               out << buffer2;

               current = instance.Pulses[current].Next;
            }
         }

         out.close();
      }
      else
      {
         printf("Read failed!\n");
      }
   }
}

int main(int argc, char ** argv)
{
   if (argc != 3)
   {
      printf("Syntax: p64conv <infile> <outfile>\n");
      exit(0);
   }

   string in  = argv[1];
   string out = argv[2];

   if (RE2::PartialMatch(in, "\\.p64") && RE2::PartialMatch(out, "\\.txt"))
      p642txt(in, out);
   else if (RE2::PartialMatch(in, "\\.txt") && RE2::PartialMatch(out, "\\.p64"))
      txt2p64(in, out);
   else
      cout << "Unknown conversion\n";
}
