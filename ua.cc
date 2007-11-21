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

// FILE COMPARISONS BY MD5 HASH VALUE - PROGRAM
//
// THE NAME ua COMES FROM UgyanAz (HUNGARIAN FOR SAME)
//
// BUILD:
// THIS TOOL REQUIRES THE OPENSSL C LIBRARIES (libcrypto).
//
// g++ -O3 -o ua filei.cc ua.cc -I . -lcrypto
// 
// or
//
// g++ -O3 -o ua filei.cc ua.cc -I . -D__NOHASH -lcrypto
// if you prefer to use tree based containers.
//
// once compiled,
//
// $ ua -vh
//
// will provide help on using the program

#if !defined(__UA_VERSION)
#define __UA_VERSION "1.0"
#endif

#include <filei.h>

static char __help[] = 
"ua [OPTION]... [FILE]...\n\n"
"where OPTION is\n" 
"  -i:         ignore case\n"
"  -w:         ignore white space\n"
"  -n:         do not ask the FS for file size\n"
"  -v:         verbose output (prints stuff to stderr), verbose help\n" 
"  -m <max>:   consider only the first <max> bytes\n"
"  -2:         perform two stage hashing\n"
"  -s <sep>:   separator (default SPACE)\n"
"  -p:         also print the hash value\n"
"  -b <bsize>: set internal buffer size (default 1024)\n"
"  -h:         this help (-vh more verbose help)\n"
"  -           read file names from stdin\n";

static char __vhelp[] =
"The algorithm performs the following steps:\n\n"
"   1. Ask the FS for file size and throw away files with unique counts\n"
"   2. If so requested, calculate a fast hash on a fixed-size prefix\n"
"      of the files with the same byte count and throw away the ones\n"
"      with unique prefix hash\n"
"   3. The still matching files will go through a full MD5 hash\n\n"
"-w implies -n, since the byte count is irrelevant information.\n"
"The two-stage hashing algorithm first calculates identical sets\n"
"considering only the first <max> bytes (thus the -2 option requires -m)\n"
"and then from these sets calculates the final result.\n"
"This can be much faster when there are many files with the same size\n"
"or when comparing files with whitespaces ignored. When -w and -m are\n"
"both set, <max> refers to the first <max> non-white characters.\n\n"
"The program returns (to the shell) 0 on success and 1 otherwise.\n\n"
"Files that cannot be processed are simply skipped (-v reports these).\n\n"
"Examples.\n\n"
"  Get help on usage.\n\n"
"    $ ua -h\n"
"    $ ua -vh\n\n"
"  Find identical files in the current directory.\n\n"
"    $ ua *\n"
"    $ ls | ua -p -\n\n"
"    In the first case, the files are read from the command line, while in\n"
"    the second the file names are read from the standard input. The letter\n"
"    one also prints the hashcode.\n\n"
"  Compare text files.\n\n"
"    $ ua -iwvb256 f1.txt f2.txt f3.txt\n\n"
"    Compares the three files ignoring letter case and white spaces.\n"
"    Intermediate steps will be reported on stderr (-v). The -w implies\n"
"    -n, thus file sizes are not grouped. The internal buffer size is\n"
"    reduced to 256, since the whitespaces will cause data to be moved\n"
"    in the buffer.\n\n"
"  Calculate the number of identical files under home.\n\n"
"    $ find ~ -type f | ua -2m256 - | wc -l\n\n"
"    Considering the large number of files, the calculation will be\n"
"    performed with a two stage hash (-2).  Only files that pass the\n"
"    256 byte prefix hash will be fully hashed.\n\n" 
"    Depending on what you compare, the fastest\n"
"    running times probably correspond to one of these option sets:\n\n"
"      (nothing):  there are many files most having unique size\n"
"      -2m256:     lots of files, many of the same size\n"
"      -2nm256:    files of the same size, or comparing files with white\n"
"                  spaces ignored\n\n"
"  Find identical header files.\n\n"
"    $ find /usr/include -name '*.h' | ua -b256 -wm256 -2s, -\n\n"
"    Ignore white spaces -w (and thus use a smaller buffer -b256). Perform\n"
"    the calculation in two stages (-2), first cluster based on the\n"
"    whitespace-free first 256 characters (-m256). Also, separate the\n"
"    identical files in the output by commas (-s,).\n\n"
"Output\n\n"
"  Each line of the output represents one set of identical files. The columns\n"
"  are the path names separated by <sep> (-s). When -p set, the first column\n"
"  will be the hash value. Remember that if -i or -w are set the hash value\n"
"  will likely be different from what md5sum would give.\n\n"
"Blame\n\n"
"  istvan.hernadvolgyi@gmail.com\n\n";


static void __phelp(bool v) {
   if (v) {
      std::cout << "Find identical sets of files." << std::endl << std::endl
                << __help << std::endl << __vhelp 
                << "Version: " << __UA_VERSION 
#if defined(__UA_USEHASH)
                << "_hash"
#else
                << "_tree"
#endif
                << std::endl << std::endl;
   } else {
      std::cout << __help << std::endl
                << "version: " << __UA_VERSION 
#if defined(__UA_USEHASH)
                << "_hash"
#else
                << "_tree"
#endif
                << std::endl << std::endl
                << "Type ua -vh for more help. If in doubt, one of " 
                << std::endl << std::endl
                << "$ find ... | au -" << std::endl
                << "$ find ... | au -2m256 -" << std::endl << std::endl;
   }
   std::cout.flush();
}

int main(int argc, char* const * argv) {

   
   fsetc_t files;

   bool ic = false; // ignore case
   bool iw = false; // ignore white space
   bool v = false; // verbose
   bool stage = false; // two stage
   int BN = 1024; // buffer size
   bool ph = false; // print hash
   bool count = true; // take size into account

   int max = 0; // max chars to consider, ALL

   bool comm = true; // from command line

   std::string sep(" "); // default sep

   if (argc <= 1) {
      __phelp(false);
      return 1;
   }

   int opt;
   while((opt = ::getopt(argc,argv,"hb:viws:m:2pn")) != -1) {
      switch(opt) {
         case 'b':
            BN = ::atoi(::optarg);
            if (!BN) {
               std::cerr << "Invalid buffer size " << ::optarg << std::endl;
               return 1;
            }
            break;
         case 'm':
            max = ::atoi(::optarg);
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
         case 's':
            sep = std::string(::optarg);
            break;
         case '2':
            stage = true;
            break;
         case 'p':
            ph = true;
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

   if (stage && !max) {
      std::cerr << "The two stage algorithm requires -m set!" << std::endl;
      return 1;
   }

   if (count && iw) count = false;

   if (count && max && !stage) count = false;

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

   char fileb[1024];

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


      try {
         size_t s = count ? filei::fsize(file) : 0;

         files[s].push_back(file);
         if (v) std::cerr << (count ? "Counting " : "Spooling ") 
                          << file << std::endl;
      } catch(const char* e) {
         if (v) std::cerr << "Skipping " << file << ", " << e << std::endl;
         continue;
      }
   }


   // iterate over size groups
   for(fsetc_t::const_iterator fct= files.begin(); fct != files.end(); ++fct) {
      if (fct->second.size() < 2) continue;

      fset_t cands(ic,iw,max,BN);

      // iterate over same size files
      for(fvec_t::const_iterator fit = fct->second.begin(); 
         fit != fct->second.end(); ++fit) {

         try {
            // add candidate file
            cands.add(*fit);
            if (v && !count) std::cerr << "Processed " << *fit << std::endl;
         } catch(const char* e) {
            if (v && !count) std::cerr << "Skipping " << *fit 
               << ", " << e <<  std::endl;
            continue;
         }
      }

      const res_t* resp = 0;
      res_t fres;
      if (stage) { // if -2
         try {
            fset_t::common(fres,cands.common(),ic,iw,0,BN);
            resp = &fres;
         } catch(const char* e) {
            if (v && !count) std::cerr << e <<  std::endl;
            continue;
         }
      } else resp = & cands.common();

      fset_t::produce(*resp,std::cout,sep,ph);
   }

   return 0;

}
