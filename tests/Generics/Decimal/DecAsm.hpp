/* 
 * This file is part of the UnixCommons distribution (https://github.com/yoori/unixcommons).
 * UnixCommons contains help classes and functions for Unix Server application writing
 *
 * Copyright (c) 2012 Yuri Kuznecov <yuri.kuznecov@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef DECASM_HPP
#define DECASM_HPP

static const uint64_t MAX64 = static_cast<uint64_t>(-1);

template <const uint64_t BASE>
uint64_t
add64(uint64_t a, uint64_t b, uint64_t& l) throw ()
{
#ifdef __x86_64__
  uint64_t rh, rl;
  __asm__ __volatile__(
    "xor %%rdx, %%rdx\n"
    "add %%rbx, %%rax\n"
    "adc $0, %%rdx\n"
    "div %%rcx\n"
    : "=a" (rh), "=d" (rl)
    : "a" (a), "b" (b), "c" (BASE)
  );
  l = rl;
  return rh;
#else
  uint64_t h = a / BASE + b / BASE;
  a %= BASE;
  b %= BASE;
  if (BASE - a <= b)
  {
    h++;
    l = b - (BASE - a); 
  }
  else
  {
    l = a + b;
  }
  return h;
#endif
}

template <const uint64_t BASE>
void
sub64(uint64_t& hi, uint64_t& lo, uint64_t h, uint64_t l) throw ()
{
  if (l > lo)
  {
    hi = hi - h - 1;
    hi += add64<BASE>(BASE - l, lo, lo);
  }
  else
  {
    hi = hi - h;
    lo = lo - l;
  }  
}

void
subq(uint64_t& h, uint64_t& l, uint64_t sh, uint64_t sl) throw ()
{
  assert(h > sh || (h == sh && l >= sl));
  if (l >= sl)
  {
    h -= sh;
    l -= sl;
  }
  else
  {
    h -= sh + 1;
    l += MAX64 - sl;
  }
}

void
mulq(uint64_t a, uint64_t b, uint64_t& h, uint64_t& l) throw ()
{
  static const uint64_t MASK = static_cast<uint32_t>(-1);
  l = (a & MASK) * (b & MASK);
  uint64_t tmp1 = (a >> 32) * (b & MASK);
  uint64_t tmp2 = (a & MASK) * (b >> 32);
  h = (a >> 32) * (b >> 32);
  h += (tmp1 >> 32) + (tmp2 >> 32);
  uint64_t tmp3 = (tmp1 & MASK) + (tmp2 & MASK);
  if (tmp3 > MASK)
  {
    h++;
  }
  tmp3 <<= 32;
  if (MAX64 - tmp3 < l)
  {
    h++;
  }
  l += tmp3;
}

void
divq(uint64_t h, uint64_t l, uint64_t d, uint64_t& q, uint64_t& r) throw ()
{
  assert(h < d);
  q = 0;
  while (h)
  {
    uint64_t guess = MAX64 / d * h;
    uint64_t tmp1, tmp2;
    mulq(guess, d, tmp1, tmp2);
    subq(h, l, tmp1, tmp2);
    q += guess;
  }
  q += l / d;
  r = l % d;
}

template <const uint64_t BASE>
void
mul64(uint64_t a, uint64_t b, uint64_t& l, uint64_t& h) throw ()
{
#ifdef __x86_64__
  uint64_t rh, rl;
  __asm__ __volatile__(
    "mulq %%rdx\n"
    "divq %%rcx\n"
    : "=a" (rh), "=d" (rl)
    : "a" (a), "d" (b), "c" (BASE)
  );
  h = rh;
  l = rl;
#else
  mulq(a, b, h, l);
  divq(h, l, BASE, h, l);
#endif
}

template <const uint64_t BASE>
uint64_t
mul64(uint64_t hi, uint64_t lo, uint64_t v, uint64_t& h, uint64_t& l)
{
  uint64_t over1, over2;
  mul64<BASE>(hi, v, h, over1);
  mul64<BASE>(lo, v, l, over2);
  return over1 + add64<BASE>(hi, over2, hi);
}

template <const uint64_t BASE>
bool
div64(uint64_t h, uint64_t l, uint64_t d, uint64_t& q)
{
#ifdef __x86_64__
  uint64_t ret;
  uint64_t rq;
  __asm__ __volatile__(
    "cmpq %%rdi, %%rax\n"
    "jae 1f\n"
    "mulq %%rcx\n"
    "addq %%rsi, %%rax\n"
    "adcq $0, %%rdx\n"
    "divq %%rdi\n"
    "movq $1, %%rdx\n"
    "jmp 2f\n"
    "1:\n"
    "xorq %%rdx, %%rdx\n"
    "2:\n"
    : "=d" (ret), "=a" (rq)
    : "a" (h), "S" (l), "c" (BASE), "D" (d)
  );
  q = rq;
  return ret;
#else
  if (h >= d)
  {
    return false;
  }
  uint64_t tmp;
  mulq(h, BASE, h, tmp);
  if (l > MAX64 - tmp)
  {
    h++;
  }
  l += tmp;
  divq(h, l, d, q, tmp);
  return true;
#endif
}

template <const uint64_t BASE>
void
div64_unsafe(uint64_t h, uint64_t l, uint64_t d, uint64_t& q, uint64_t& r)
{
#ifdef __x86_64__
  uint64_t rq, rr;
  __asm__ __volatile__(
    "mulq %%rcx\n"
    "addq %%rsi, %%rax\n"
    "adcq $0, %%rdx\n"
    "divq %%rdi\n"
    : "=d" (rr), "=a" (rq)
    : "a" (h), "S" (l), "c" (BASE), "D" (d)
  );
  q = rq;
  r = rr;
#else
  uint64_t tmp;
  mulq(h, BASE, h, tmp);
  if (l > MAX64 - tmp)
  {
    h++;
  }
  l += tmp;
  divq(h, l, d, q, r);
#endif
}

#endif
