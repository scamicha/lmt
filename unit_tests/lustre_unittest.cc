// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// A sample program demonstrating using Google C++ testing framework.
//
// Author: wan@google.com (Zhanyong Wan)


// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.
//
// Writing a unit test using Google C++ testing framework is easy as 1-2-3:


// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <dlfcn.h>
#include <cerebro/cerebro_metric_module.h>
#include <cerebro/cerebro_monitor_module.h>

extern "C" {
#include "list.h"
#include "hash.h"
#include "proc.h"
#include "lustre.h"
#include "cmt.h"
}
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>


TEST(CMT, lmt_cmt_string_v1) {
	pctx_t context = proc_create("procmock");

	char b2[1000];
	lmt_cmt_string_v1(context, b2, 999);
	EXPECT_STREQ("1;EXPORT1;11;13;EXPORT2;11;13;EXPORT3;11;13;EXPORT1;11;13;EXPORT2;11;13;EXPORT3;11;13;EXPORT1;11;13;EXPORT2;11;13;EXPORT3;11;13;", b2);
}

struct cerebro_metric_module_info * load_plugin(std::string str)
{
   void *lib_handle;
	struct cerebro_metric_module_info *fn;
   char *error;

   lib_handle = dlopen(str.c_str(), RTLD_LAZY);
   if (!lib_handle) 
   {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
   }

   fn = (struct cerebro_metric_module_info *)dlsym(lib_handle, "metric_module_info");
   if ((error = dlerror()) != NULL)  
   {
      fprintf(stderr, "%s\n", error);
      exit(1);
   }

//   dlclose(lib_handle);
   return fn;
}

const std::string metric_module = "/media/sf_LMT_Shared/lmt/cerebro/metric/.libs/cerebro_metric_lmt_cmt.so";
const std::string monitor_module = "/media/sf_LMT_Shared/lmt/cerebro/monitor/.libs/cerebro_monitor_lmt_mysql.so";

TEST(CerebroMetric, lmt) {
   
   int period;
   struct cerebro_metric_module_info * lmt = load_plugin(metric_module.c_str());
   lmt->get_metric_period(&period);
   
   EXPECT_EQ( period, 30 );
   EXPECT_STREQ( lmt->get_metric_name(), "lmt_cmt");
}

#if 0
// Can't figure out how proc_lustre_ostlist works in here. It doesn't seem to take any kind of source directory
TEST(CerebroMetric, cmt) {
	struct cerebro_metric_module_info * cmt = load_plugin(metric_module.c_str());
	int period;
   cmt->get_metric_period(&period);
   
   EXPECT_EQ( 30, period );
   EXPECT_STREQ( "lmt_cmt", cmt->get_metric_name());
   
   unsigned int a, b;
   void *c = NULL;
   
   int ret = cmt->get_metric_value(&a,&b,&c);
   EXPECT_EQ(0, ret);
   EXPECT_STREQ( "10.10.0.100@tcp;0;0;10.10.0.103@tcp;0;0;10.10.0.104@tcp;0;4194304;", (char *)c );
}
#endif

TEST( CerebroMonitor, cmt )
{
	struct cerebro_monitor_module_info *fn;
	char * error;
   void *lib_handle = dlopen(monitor_module.c_str(), RTLD_LAZY);
   if (!lib_handle) 
   {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
   }

  fn = (struct cerebro_monitor_module_info *)dlsym(lib_handle, "monitor_module_info");

   //fn = (myfunc)dlsym(lib_handle, "lmt_db_insert_cmt_v1");
      if ((error = dlerror()) != NULL)  
   {
      fprintf(stderr, "%s\n", error);
      exit(1);
   }
   EXPECT_TRUE( fn != NULL);
   
   char buf[100];
   strcpy( buf, "1;ossname;10.10.0.100@tcp;1;3;10.10.0.103@tcp;5;7;10.10.0.104@tcp;11;4194304;" );
   
   fn->metric_update("foo", "lmt_cmt", 5, 10, (void *)buf);
}

/* Compile time mocks */
pctx_t proc_create(const char *root)
{
}
//int proc_lustre_oscinfo(pctx_t ctx, char *name, char **uuidp, char **statep)
//{
//}
int proc_lustre_ost_exportlist(pctx_t ctx, char *name, List *lp)
{
	printf("Mock proc_lustre_ost_exportlist\n");
	List l = list_create((ListDelF)free);
	list_append(l, strdup("EXPORT1"));
	list_append(l, strdup("EXPORT2"));
	list_append(l, strdup("EXPORT3"));
	*lp = l;
	return 0;
}
int proc_lustre_uuid(pctx_t ctx, char *name, char **uuidp)
{
}
int proc_lustre_ostlist(pctx_t ctx, List *lp)
{
	printf("Mock proc_lustre_ostlist\n");
	List l = list_create((ListDelF)free);
	list_append(l, strdup("OST1"));
	list_append(l, strdup("OST2"));
	list_append(l, strdup("OST3"));
	*lp = l;
	return 0;
}
int proc_lustre_get_read_and_write_bytes(pctx_t ctx, char *mdt_name, char* node_name, uint64_t* read_bytes, uint64_t* write_bytes)
{
	*read_bytes = 11;
	*write_bytes = 13;
	return 0;
}