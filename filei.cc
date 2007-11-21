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

// FILE COMPARISONS BY MD5 HASH VALUE - IMPLEMENTATION
//

#include <filei.h>

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
}

#include <fstream>

void* (*filei::_gbuff)(size_t) = &filei::gbuff;
size_t (*filei::_buffc)() = &filei::buffc;
void (*filei::_relbuff)(void*) = 0;
char filei::_buffer[__UABUFFSIZE];
     
filei::filei(const std::string& path, bool ic, bool iw, int max, size_t bs)
throw(const char*):_path(path),_h(0)  {
   ::bzero(_md5,16); // zero out
   calc(ic,iw,bs,max);
}

// in-place turn buffer into lower case
static void __lower_case(char* buffer, int n) {
   for(char *p = buffer; p < buffer + n; ++p) 
      if (*p <= 'Z' && *p >= 'A') *p += 'a' - 'A';
}

// count contiguous white spaces from pointer
static int __countw(const char* p, const char* e) {
   int w = 0;
   for(;p<e; ++p) {
      switch(*p) {
         case ' ':
         case '\t':
         case '\r':
         case '\n':
            ++w;
            break;
         default:
            return w;
      }
   }
   return w;
}

// in-place remove contiguous white space, return number of chars removed
static int __remove_white(char* buffer, int n) {
   int r = 0;

   for(char *p = buffer; p < buffer + n; ++p) {
      int k = __countw(p,buffer+n);
      if (k) {
         // shift by k
         for(char* pk = p; k && pk < buffer + n - k; ++pk) *pk = *(pk+k);
         n -= k;
         r += k;
         ++p; // next cannot be white
      }
   }

   return r;
}

void filei::calc(bool ic, bool iw, size_t bn, int max) throw(const char*) {
   
   const char* error = 0;

   char* buffer = 0;
   
   try {
      buffer= static_cast<char*>((*_gbuff)(bn));   // get buffer
      if (!buffer) throw 1;
   } catch(...) {
      throw "Could not allocate memory";
   }

   bn = _buffc ? std::min(bn,(*_buffc)()) : bn;  // get buffer size

   int tot = 0;

   std::ifstream is(_path.c_str());
   if (!is.good()) { error = "Could not open file"; goto FINALLY; }

   MD5_CTX ctxt;
   if (!MD5_Init(&ctxt)) { error = "Could not init MD5"; goto FINALLY; }


   for(bool done=false;!done;) {
      is.read(buffer,bn);
      int n = is.gcount();
      if (!n) break;

      if (ic) __lower_case(buffer,n);
      if (iw) {
         n -=  __remove_white(buffer,n);
         if (!n) continue;
      }

      if (max) {
         if (tot + n > max) {
            n = max - tot;
            done = true;
         } else tot += n;
      }

      if (!MD5_Update(&ctxt,buffer,n)) { error = "MD5 calc error"; goto FINALLY; }
      if (is.eof()) break;
   }

   if (!MD5_Final(_md5,&ctxt)) { error= "MD5 calc error (final)"; goto FINALLY; }

   for(int i = 0, s = 0; i < 16; ++i, ++s) {
      if (s >= (int)sizeof(size_t)) s = 0;
      _h ^= ((size_t)_md5[i]) << (s << 3);
   }

FINALLY:

   // clean-up
   is.close();
   if (_relbuff) (*_relbuff)(buffer);

   if (error) throw error;
}


off_t filei::fsize(const std::string& path) throw(const char*) {
   struct stat fsi;

   if (::stat(path.c_str(),&fsi)) throw "Could not stat file.";
   //if (!S_ISREG(fsi.st_mode) && !S_ISLNK(fsi.st_mode)) throw "Not a file.";
   return fsi.st_size;
}

bool filei::md5cmp::operator()(const filei& fi1, const filei& fi2) const {
   if (fi1.h() < fi2.h()) return true;
   else if (fi1.h() > fi2.h()) return false;
   for(const unsigned char* p1=fi1._md5, *p2=fi2._md5;
      p1< fi1._md5 + 16; ++p1,++p2) {
      if (*p1 < *p2) return true;
      else if (*p1 > *p2) return false;
   }
   return false;
}


bool filei::md5eq::operator()(const filei& fi1, const filei& fi2) const {
   if (fi1.h() != fi2.h()) return false;
   for(const unsigned char* p1=fi1._md5, *p2=fi2._md5;
      p1< fi1._md5 + 16; ++p1,++p2) {
      if (*p1 != *p2) return false;
   }
   return true;
}


