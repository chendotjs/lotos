#include "../ssstr.h"
#include "minctest.h"
#include <stdio.h>
#include <string.h>

void test1() {
  ssstr_t s = SSSTR("hello");
  lequal(5, s.len);
  lsequal("hello", s.str);
}

void test2() {
  ssstr_t s1 = SSSTR("hello");
  ssstr_t s2 = SSSTR("hello");
  lequal(0, ssstr_cmp(&s1, &s2));

  s1 = SSSTR("");
  s2 = SSSTR("");
  lequal(0, ssstr_cmp(&s1, &s2));
}

void test3() {
  char str1[] = "hello";
  char str2[] = "hello";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);
  lequal(0, ssstr_cmp(&s1, &s2));
}

void test4() {
  char str1[] = "hello";
  char str2[] = "hullo";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);
  lequal(-1, ssstr_cmp(&s1, &s2));
}

void test5() {

  char str1[] = "hello_world";
  char str2[] = "hello";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);

  lequal(1, ssstr_cmp(&s1, &s2));
  lequal(11, s1.len);
  lequal(5, s2.len);
}

void test6() {
  char str1[] = "hello_world";
  char str2[] = "hullo";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);
  lequal(-1, ssstr_cmp(&s1, &s2));
}

void test7() {
  char str1[] = "";
  char str2[] = "";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);

  lok(s1.str != s2.str);
  lequal(0, ssstr_cmp(&s1, &s2));
}

void test8() {
  char str1[] = "hello";
  char str2[] = "hello";

  ssstr_t s1 = SSSTR(str1);

  lok(ssstr_equal(&s1, str2));
  lok(ssstr_equal(&s1, "hello"));
  lok(!ssstr_equal(&s1, "hello_world"));
  lok(!ssstr_equal(&s1, "hullo"));
}

void test9() {
  char str1[] = "HellO";
  char str2[] = "heLLo-World";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);

  ssstr_tolower(&s1);
  ssstr_tolower(&s2);

  lok(ssstr_equal(&s1, "hello"));
  lok(ssstr_equal(&s2, "hello-world"));
}

void test10() {
  char str1[] = "HellO";
  char str2[] = "heLLo-World";

  ssstr_t s1 = SSSTR(str1);
  ssstr_t s2 = SSSTR(str2);

  lok(ssstr_caseequal(&s1, "hello"));
  lok(ssstr_caseequal(&s2, "hello-world"));
}

int main(int argc, char const *argv[]) {
  lrun("test1", test1);
  lrun("test2", test2);
  lrun("test3", test3);
  lrun("test4", test4);
  lrun("test5", test5);
  lrun("test6", test6);
  lrun("test7", test7);
  lrun("test8", test8);
  lrun("test9", test9);
  lrun("test10", test10);
  lresults();
  printf("\n\n");
  return lfails != 0;
}
