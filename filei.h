// FILE COMPARISONS BY MD5 HASH VALUE - HEADER
//
// (c) Dr. I. T. Hernadvolgyi, EU.EDGE LLC, 2007
//
// THIS IS FREE SOFTWARE 
//
// FOR DETAILS SEE http://www.gnu.org/licenses/lgpl-3.0.txt
//
//
 
#if !defined(_FILEI_H_)
#define _FILEI_H_


// decide whether to prefer hashed containers
// first you need gcc 3.x or higher 
// second __NOHASH should be undefined
//
#if defined(__GNUC__) && !defined(__NOHASH)
#if __GNUC__ >= 3
#define __UA_USEHASH
#endif
#endif

#include <string>

#if defined(__UA_USEHASH)
#include <ext/hash_set>
#include <ext/hash_map>
#endif

#include <set>
#include <map>

#include <vector>

#include <iostream>
#include <iomanip>

/** File info.
 *
 * Contains the path name and the corresponding md5 hash. 
 */
class filei {
   private:
      static char _buffer[32768];

      std::string _path; // path name
      unsigned char _md5[16]; // md5 hash
      size_t _h; // hash of hash :)

      // calculate hash
      void calc(bool ic, bool iw, int bs, int max) throw(const char*); 

      static char* gbuff(size_t) { return _buffer; }
      static int buffc() { return 32768; }


   public:

      /** Constructor.
       *
       * The constructor will open the file and calculate the hash.
       *
       * @param path file name
       * @param ic ignore case
       * @param iw ignore white space (in essence, remove it)
       * @param max consider at most these many bytes for the hash (0: ALL)
       * @param bs buffer size of internal work buffer (default 1024)
       * @throws an error message if construction failed
       */
      filei(const std::string& path, bool ic, bool iw, int max=0, int bs=1024)
      throw(const char*);

      /** Get an md5 hash char.
       * @param i index
       * @return md5 hash char at index
       */
      unsigned char operator[](int i) const { return _md5[i]; }

      /** Get the md5 hash characters.
       * @return the md5 hash characters. 
       */
      const unsigned char* md5() const { return _md5; }

      /** Get path name.
       * @return path name
       */
      const std::string& path() const { return _path; }

      /** Get hash code.
        * This is a hash of the _md5 value and meant to be
        * used in hashed data structures.
        * @return hash code
        */
      const size_t h() const { return _h; }


      static off_t fsize(const std::string& path) throw(const char*);

      /** Functor for hashed containers.
       */
      struct md5hash {

         /** Calculate a hash of the md5 hash.
          * @param fi file info
          * @return hash from md5
          */
         size_t operator()(const filei& fi) const { return fi.h(); }
      };

      /** Functor for sorted containers.
       */
      struct md5cmp {

         /** Compare the md5 hashes numerically.
           * @param fi1 file info 1
           * @param fi2 file info 2
           * @return true if fi1._md5 < fi2._md5
           */
         bool operator()(const filei& fi1, const filei& fi2) const;
      };


      /** Functor for containers.
        */
      struct md5eq {
    
         /** Compare the md5 hashes numerically.
           * @param fi1 file info 1
           * @param fi2 file info 2
           * @return true if fi1._md5 == fi2._md5
           */
         bool operator()(const filei& fi1, const filei& fi2) const;
      };

      // assign the three plugins below differently 
      // if you want re-entrant calculations,
      // or different buffer allocation
      // by default, all calculations share the same buffer

      /** Function that gets work buffer.
       * The size_t argument is the requested capacity.
       */
      static char* (*_gbuff)(size_t);

      /** Function that tells work buffer capacity. */
      static int (*_buffc)();

      /** Function that releases work buffer. */
      static void (*_relbuff)(char*);
     

};


/** Vector of file names. */
typedef std::vector<std::string> fvec_t;

/** Set of file infos.
  */
typedef std::set<filei,filei::md5cmp> set_t;

/** Map from file name to file names. 
 * The key and the values together make up a set of 
 * identical files.
 */
typedef std::map<filei,fvec_t,filei::md5cmp> map_t;


#if defined(__UA_USEHASH)

/** Hashed set of file infos.
  */
typedef __gnu_cxx::hash_set<filei,filei::md5hash,filei::md5eq> hset_t;

/** Hashed map from file name to file names. 
 * The key and the values together make up a set of 
 * identical files.
 */
typedef __gnu_cxx::hash_map<filei,fvec_t,filei::md5hash,filei::md5eq> hmap_t;
#endif

/** Sets of identical files.
 *
 * S: must be some set<filei>
 * M: must be some map<filei,std::vector<std::string> >
 */
template<typename S, typename M>
class fset {
   private:

      // "heads" are random representatives of sets of files
      // with the same _md5 

      S _files; // set of file "heads": each has unique _md5
      M _cmn;   // the subsets that are identical ("head" -> path names)

      bool _ic; // ignore case
      bool _iw; // ignore whitespace
      int _max; // max chars to consider
      int _bs;  // buffer size

      typedef typename M::const_iterator it_t; // subset iterator

      /** Add a file info.
        * @param fi file info
        */
      void add(const filei& fi) {
         typename S::const_iterator i = _files.find(fi);
         if (i != _files.end()) _cmn[*i].push_back(fi.path());
         else _files.insert(fi);
      }


   public:
 
      /** Constructor.
       *
       * @param ic ignore case
       * @param iw ignore white space
       * @param max consider at most these many bytes for hash (0: ALL)
       * @param bs internal buffer size (default 1024)
       */
      fset(bool ic, bool iw, int max = 0, int bs = 1024):
         _ic(ic), _iw(iw), _max(max), _bs(bs) {
      }

 
      /** Add a file.
        * @param path file
        * @throws a description if hash could not be constructed
        */
      void add(const std::string& path) throw(const char*) {
         add(filei(path,_ic,_iw,_max,_bs));
      }

      /** Print the sets of identical files.
        *
        * Each set of identical files are printed on a single line.
        * @param cmn common files
        * @param os output stream
        * @param s separator (default " ")
        * @param ph print hash (if true, the first column is the hash)
        */
      static void produce(const M& cmn, std::ostream& os,
         const std::string& s = " ", bool ph = false) {

         for(it_t it= cmn.begin(); it != cmn.end(); ++it) {
            if (ph) { // print hash
               for(int i=0; i< 16; ++i) {
                  int hi = it->first[i] >> 4 & 0x0f;
                  int lo = it->first[i] & 0x0f;
                  os << std::hex << hi << lo;
               }
               os << s;
            }
            os << it->first.path();
            for(int i=0; i<(int)it->second.size();++i) os << s << it->second[i];
            os << std::endl;
         }
      }

      /** Identical sets of files.
        *
        * The result is a map from a file info to vectors of file paths.
        * The key and its corresponding values together make up one 
        * identical set of files. The key is not repeated in the values and
        * its designation as key is arbitrary. 
        *
        * @return a map of the identical sets
        */
      const M& common() const { return _cmn; }

      /** Calculate sets of identical files.
       *
       * This function can be used to implement a multi-stage
       * calculation of identical file sets. The param cmn
       * could be obtained by looking at only the prefix of the files
       * and thus files in its identical sets may not be totally identical
       * (only in prefix). However, files which are not in these sets can
       * be safely ignored.
       *
       * @param res resulting common set of files (returned)
       * @param cmn common set of files
       * @param ic ignore case
       * @param iw ignore white space
       * @param max consider at most these many bytes for hash (0: ALL)
       * @param bs internal buffer size (default 1024)
       */
      static void common(M& res, const M& cmn, 
         bool ic, bool iw, int max=0, int bs=1204) {
         for(it_t it=cmn.begin(); it != cmn.end(); ++it) {
            fset files(ic,iw,max,bs);
            files.add(it->first.path());
            for(int i=0; i<(int)it->second.size();++i) files.add(it->second[i]);

            const M& locmn = files.common();
            for(it_t lit = locmn.begin(); lit!=locmn.end(); ++lit) {
               res[lit->first] = lit->second;
            }
         }
      }
};

/** Choose preferred types for file set and result set.
 * fsetc_t: preferred type for the map from file size to file names
 * res_t:   preferred type for the map of identical subsets 
 * fset_t:  preferred type for the fset object
 *
 */
#if defined(__UA_USEHASH)
typedef fset<hset_t,hmap_t> fset_t;
typedef hmap_t res_t;
typedef __gnu_cxx::hash_map<size_t,fvec_t> fsetc_t;
#else
typedef std::map<size_t,fvec_t> fsetc_t;
typedef map_t res_t;
typedef fset<set_t,map_t> fset_t;
#endif

#endif
