/**
 * @file unc_text.cpp
 * A simple class that handles the chunk text.
 *
 * @author  Ben Gardner
 * @license GPL v2+
 */
#include "unc_text.h"
#include "unc_ctype.h"
#include "unicode.h" // encode_utf8()
#include <algorithm>

using namespace std;


static size_t fix_len_idx(size_t size, size_t idx, size_t len);


static size_t fix_len_idx(size_t size, size_t idx, size_t len)
{
   if (idx >= size)
   {
      return(0);
   }

   const size_t left = size - idx;
   return((len > left) ? left : len);
}


void unc_text::update_logtext()
{
   if (m_logok)
   {
      return;
   }

   // make a pessimistic guess at the size
   m_logtext.clear();
   m_logtext.reserve(m_chars.size() * 3);
   for (int m_char : m_chars)
   {
      if (m_char == '\n')
      {
         m_char = 0x2424; // NL symbol
      }
      else if (m_char == '\r')
      {
         m_char = 0x240d; // CR symbol
      }
      encode_utf8(m_char, m_logtext);
   }
   m_logtext.push_back(0);
   m_logok = true;
}


int unc_text::compare(const unc_text &ref1, const unc_text &ref2, size_t len)
{
   const size_t len1    = ref1.size();
   const size_t len2    = ref2.size();
   const auto   max_idx = std::min({ len, len1, len2 });
   size_t       idx     = 0;

   for ( ; idx < max_idx; idx++)
   {
      // exactly the same character ?
      if (ref1.m_chars[idx] == ref2.m_chars[idx])
      {
         continue;
      }

      int diff = unc_tolower(ref1.m_chars[idx]) - unc_tolower(ref2.m_chars[idx]);
      if (diff == 0)
      {
         /*
          * if we're comparing the same character but in different case
          * we want to favor lower case before upper case (e.g. a before A)
          * so the order is the reverse of ASCII order (we negate).
          */
         return(-(ref1.m_chars[idx] - ref2.m_chars[idx]));
      }

      // return the case-insensitive diff to sort alphabetically
      return(diff);
   }

   if (idx == len)
   {
      return(0);
   }

   // underflow save: return(len1 - len2);
   return((len1 > len2) ? (len1 - len2) : -static_cast<int>(len2 - len1));
}


bool unc_text::equals(const unc_text &ref) const
{
   const size_t len = size();

   if (ref.size() != len)
   {
      return(false);
   }

   for (size_t idx = 0; idx < len; idx++)
   {
      if (m_chars[idx] != ref.m_chars[idx])
      {
         return(false);
      }
   }
   return(true);
}


const char *unc_text::c_str()
{
   update_logtext();
   return(reinterpret_cast<const char *>(&m_logtext[0]));
}


void unc_text::set(int ch)
{
   m_chars.clear();
   m_chars.push_back(ch);
   m_logok = false;
}


void unc_text::set(const unc_text &ref)
{
   m_chars = ref.m_chars;
   m_logok = false;
}


void unc_text::set(const unc_text &ref, size_t idx, size_t len)
{
   m_logok = false;

   const auto ref_size = ref.size();

   if (len == ref_size)
   {
      m_chars = ref.m_chars;
      return;
   }

   m_chars.resize(len);

   len = fix_len_idx(ref_size, idx, len);
   for (size_t di = 0;
        len > 0;
        di++, idx++, len--)
   {
      m_chars[di] = ref.m_chars[idx];
   }
}


void unc_text::set(const string &ascii_text)
{
   const size_t len = ascii_text.size();

   m_chars.resize(len);
   for (size_t idx = 0; idx < len; idx++)
   {
      m_chars[idx] = ascii_text[idx];
   }
   m_logok = false;
}


void unc_text::set(const char *ascii_text)
{
   const size_t len = strlen(ascii_text);

   m_chars.resize(len);
   for (size_t idx = 0; idx < len; idx++)
   {
      m_chars[idx] = *ascii_text++;
   }
   m_logok = false;
}


void unc_text::set(const value_type &data, size_t idx, size_t len)
{
   m_chars.resize(len);

   len = fix_len_idx(data.size(), idx, len);
   for (size_t di = 0;
        len > 0;
        di++, idx++, len--)
   {
      m_chars[di] = data[idx];
   }

   m_logok = false;
}


void unc_text::resize(size_t new_size)
{
   if (size() == new_size)
   {
      return;
   }

   m_chars.resize(new_size);
   m_logok = false;
}


void unc_text::clear()
{
   m_chars.clear();
   m_logok = false;
}


void unc_text::insert(size_t idx, int ch)
{
   m_chars.insert(m_chars.begin() + idx, ch);
   m_logok = false;
}


void unc_text::insert(size_t idx, const unc_text &ref)
{
   m_chars.insert(m_chars.begin() + idx, ref.m_chars.begin(), ref.m_chars.end());
   m_logok = false;
}


void unc_text::append(int ch)
{
   m_chars.push_back(ch);
   m_logok = false;
}


void unc_text::append(const unc_text &ref)
{
   m_chars.insert(m_chars.end(), ref.m_chars.begin(), ref.m_chars.end());
   m_logok = false;
}


void unc_text::append(const string &ascii_text)
{
   unc_text tmp(ascii_text);

   append(tmp);
}


void unc_text::append(const char *ascii_text)
{
   unc_text tmp(ascii_text);

   append(tmp);
}


void unc_text::append(const value_type &data, size_t idx, size_t len)
{
   unc_text tmp(data, idx, len);

   append(tmp);
}


bool unc_text::startswith(const char *text, size_t idx) const
{
   const auto orig_idx = idx;

   for ( ; idx < size() && *text; idx++, text++)
   {
      if (*text != m_chars[idx])
      {
         return(false);
      }
   }

   return(idx != orig_idx && (*text == 0));
}


bool unc_text::startswith(const unc_text &text, size_t idx) const
{
   size_t     si       = 0;
   const auto orig_idx = idx;

   for ( ; idx < size() && si < text.size(); idx++, si++)
   {
      if (text.m_chars[si] != m_chars[idx])
      {
         return(false);
      }
   }

   return(idx != orig_idx && (si == text.size()));
}


int unc_text::find(const char *text, size_t sidx) const
{
   const size_t len = strlen(text); // the length of 'text' we are looking for
   const size_t si  = size();       // the length of the string we are looking in

   if (si < len)                    // not enough place for 'text'
   {
      return(-1);
   }

   const size_t midx = si - len;
   for (size_t idx = sidx; idx <= midx; idx++)
   {
      bool match = true;
      for (size_t ii = 0; ii < len; ii++)
      {
         if (m_chars[idx + ii] != text[ii])
         {
            match = false;
            break;
         }
      }
      if (match) // 'text' found at position 'idx'
      {
         return(idx);
      }
   }
   return(-1);  //  'text' not found
}


int unc_text::rfind(const char *text, size_t sidx) const
{
   size_t len  = strlen(text);
   size_t midx = size() - len;

   if (sidx > midx)
   {
      sidx = midx;
   }

   for (size_t idx = sidx; idx != 0; idx--)
   {
      bool match = true;
      for (size_t ii = 0; ii < len; ii++)
      {
         if (m_chars[idx + ii] != text[ii])
         {
            match = false;
            break;
         }
      }
      if (match)
      {
         return(idx);
      }
   }
   return(-1);
}


void unc_text::erase(size_t idx, size_t len)
{
   if (len == 0)
   {
      return;
   }

   m_chars.erase(m_chars.begin() + idx, m_chars.begin() + idx + len);
}


int unc_text::replace(const char *oldtext, const unc_text &newtext)
{
   const size_t olen         = strlen(oldtext);
   const size_t newtext_size = newtext.size();

   int          fidx = find(oldtext);
   int          rcnt = 0;

   while (fidx >= 0)
   {
      rcnt++;
      erase(static_cast<size_t>(fidx), olen);
      insert(static_cast<size_t>(fidx), newtext);

      fidx = find(oldtext, static_cast<size_t>(fidx) + newtext_size - olen + 1);
   }
   return(rcnt);
}
