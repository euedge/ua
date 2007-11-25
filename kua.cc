/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code was developed for an EU.EDGE internal project and
 * is made available according to the terms of this license.
 * 
 * The Initial Developer of the Original Code is Istvan T. Hernadvolgyi,
 * EU.EDGE LLC.
 *
 * Portions created by EU.EDGE LLC are Copyright (C) EU.EDGE LLC.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License (the "GPL"), in which case the
 * provisions of GPL are applicable instead of those above.  If you wish
 * to allow use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * License, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the License or the GPL.
 */

// Look for a file which is the "same" as this
//
// THE NAME kua COMES FROM Keresd UgyanAz (Hungarian for Find the Same)
//
// BUILD:
//
// g++ -O3 -o kua filei.cc kua.cc -I . 
// 
// $ kua -vh
//
// will provide help on using the program

#if !defined(__KUA_VERSION)
#define __KUA_VERSION "1.0"
#endif

#include <filei.h>

extern "C" {
#include <stdio.h>
}

static char __help[] = 
"kua [OPTION]... [FILE]...\n\n"
"where OPTION is\n" 
"  -f <file>:  file to compare to\n"
"  -i:         ignore case\n"
"  -w:         ignore white space\n"
"  -n:         do not ask the FS for file size\n"
"  -v:         verbose output (prints stuff to stderr), verbose help\n" 
"  -b <bsize>: set internal buffer size (default 1024)\n"
"  -h:         this help (-vh more verbose help)\n"
"  -           read file names from stdin\n";

static char __vhelp[] =
"kua looks for files which are identical to the one given as the argument "
"of -f. For example, \n\n"
"  $ kua -f f.txt `ls`\n\n"
"looks for files identical to f.txt in the current directory, while\n\n"
"  $ find ~ -type f | kua -f f.txt -\n\n"
"will compare f.txt to each file under home.\n"
"Blame\n\n"
"  istvan.hernadvolgyi@gmail.com\n\n";


static void __phelp(bool v) {
   if (v) {
      std::cout << "Find files identical to the given one." 
                << std::endl << std::endl
                << __help << std::endl << __vhelp 
                << "Version: " << __KUA_VERSION 
                << std::endl << std::endl;
   } else {
      std::cout << __help << std::endl
                << "version: " << __KUA_VERSION 
                << std::endl << std::endl
                << "Type kua -vh for more help. If in doubt,"
                << std::endl << std::endl
                << "$ find ... | kua -f <file> -" << std::endl;
   }
   std::cout.flush();
}

int main(int argc, char* const * argv) {

   
   std::string cfile;

   bool ic = false; // ignore case
   bool iw = false; // ignore white space
   bool v = false; // verbose
   int BN = 1024; // buffer size
   bool count = true; // take size into account

   bool comm = true; // from command line

   if (argc <= 1) {
      __phelp(false);
      return 1;
   }

   int opt;
   while((opt = ::getopt(argc,argv,"f:hb:viws:m:n")) != -1) {
      switch(opt) {
         case 'f':
            cfile = std::string(::optarg);
            break;
         case 'b':
            BN = ::atoi(::optarg);
            if (!BN) {
               std::cerr << "Invalid buffer size " << ::optarg << std::endl;
               return 1;
            }
            break;
         case 'i':
            ic = true;
            break;
         case 'v':
            v = true;
            break;
         case 'w':
            iw = true;
            break;
         case 'n':
            count = false;
            break;
         case 'h':
            __phelp(v);
            return 0;
         case '?':
            std::cerr << "Type " << argv[0] << " -h for options." << std::endl;
            return 1;
      }
   }

   if (!cfile.size()) {
      std::cerr << "File param missing. See kua -vh" << std::endl;
      return 1;
   }

   if (count && iw) count = false;

   if (argc > ::optind) { 
      if (argc >= ::optind +1 && *argv[::optind] == '-') {
         if (argc > ::optind + 1) {
            std::cerr << "Spurious arguments!" << std::endl;
            return 1;
         }
         ++optind;
         comm = false; // read files names from stdin
      }
   }

   char fileb[FILENAME_MAX];

   size_t n = 0;

   if (count) {
      try {
         n = filei::fsize(cfile);
      } catch(const char *e) {
         std::cerr << e << std::endl;
      }
   }

   for(int i = ::optind;;) {
      char* file;
      if (comm) {
         if (i == argc) break;
         file = argv[i++];
      } else {
         std::cin.getline(fileb,1024);
         if (std::cin.eof()) break;
         file = fileb;
      }


      if (v) std::cerr << "Considering " << file << std::endl;
      try {
         if (count) if (n != filei::fsize(file)) continue;
         if (filei::eq(cfile,file,ic,iw,0,BN)) std::cout << file << std::endl;
      } catch(const char* e) {
         if (v) std::cerr << "Skipping " << file << ", " << e << std::endl;
         continue;
      }
   }


   return 0;

}
