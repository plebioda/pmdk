/*
 * Copyright 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * file_core_rw.c -- unit test for pmemfile_read & pmemfile_write
 */

#include "unittest.h"
#include "pmemfile_test.h"

static void
test1(PMEMfilepool *pfp)
{
	PMEMfile *f = PMEMFILE_OPEN(pfp, "/file1", O_CREAT | O_EXCL | O_WRONLY,
			0644);

	_pmemfile_list_root(pfp, "/file1 0");

	const char *data = "Marcin S";
	char data2[4096];
	char bufFF[4096], buf00[4096];
	int len = strlen(data) + 1;
	memset(bufFF, 0xff, sizeof(bufFF));
	memset(buf00, 0x00, sizeof(buf00));

	PMEMFILE_WRITE(pfp, f, data, len, len);

	_pmemfile_list_root(pfp, "/file1 9");

	/* file is opened write-only */
	PMEMFILE_READ(pfp, f, data2, len, -1, EBADF);
	PMEMFILE_CLOSE(pfp, f);

	f = PMEMFILE_OPEN(pfp, "/file1", O_RDONLY);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, len, len);
	UT_ASSERTeq(memcmp(data, data2, len), 0);
	UT_ASSERTeq(memcmp(data2 + len, bufFF, sizeof(data2) - len), 0);

	/* file is opened read-only */
	PMEMFILE_WRITE(pfp, f, data, len, -1, EBADF);

	memset(data2, 0, sizeof(data2));
	/* end of file */
	PMEMFILE_READ(pfp, f, data2, len, 0);
	PMEMFILE_CLOSE(pfp, f);


	f = PMEMFILE_OPEN(pfp, "/file1", O_RDONLY);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), len);
	UT_ASSERTeq(memcmp(data, data2, len), 0);
	UT_ASSERTeq(memcmp(data2 + len, bufFF, sizeof(data2) - len), 0);

	PMEMFILE_CLOSE(pfp, f);


	f = PMEMFILE_OPEN(pfp, "/file1", O_RDONLY);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, 5, 5);
	UT_ASSERTeq(memcmp(data, data2, 5), 0);
	UT_ASSERTeq(memcmp(data2 + 5, bufFF, sizeof(data2) - 5), 0);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, 15, 4);
	UT_ASSERTeq(memcmp(data + 5, data2, 4), 0);
	UT_ASSERTeq(memcmp(data2 + 4, bufFF, sizeof(data2) - 4), 0);

	PMEMFILE_CLOSE(pfp, f);


	f = PMEMFILE_OPEN(pfp, "/file1", O_RDWR);

	PMEMFILE_WRITE(pfp, f, "pmem", 4, 4);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), 5);
	UT_ASSERTeq(memcmp(data + 4, data2, 5), 0);
	UT_ASSERTeq(memcmp(data2 + 5, bufFF, sizeof(data2) - 5), 0);

	PMEMFILE_CLOSE(pfp, f);


	_pmemfile_list_root(pfp, "/file1 9");


	f = PMEMFILE_OPEN(pfp, "/file1", O_RDWR);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), 9);
	UT_ASSERTeq(memcmp("pmem", data2, 4), 0);
	UT_ASSERTeq(memcmp(data + 4, data2 + 4, 5), 0);
	UT_ASSERTeq(memcmp(data2 + 9, bufFF, sizeof(data2) - 9), 0);

	PMEMFILE_CLOSE(pfp, f);


	f = PMEMFILE_OPEN(pfp, "/file1", O_RDWR);
	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 0);
	PMEMFILE_LSEEK(pfp, f, 3, SEEK_CUR, 3);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), 6);
	UT_ASSERTeq(memcmp("min S\0", data2, 6), 0);
	UT_ASSERTeq(memcmp(data2 + 6, bufFF, sizeof(data2) - 6), 0);

	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 9);
	PMEMFILE_LSEEK(pfp, f, -7, SEEK_CUR, 2);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), 7);
	UT_ASSERTeq(memcmp("emin S\0", data2, 7), 0);
	UT_ASSERTeq(memcmp(data2 + 7, bufFF, sizeof(data2) - 7), 0);

	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 9);


	PMEMFILE_LSEEK(pfp, f, -3, SEEK_END, 6);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), 3);
	UT_ASSERTeq(memcmp(" S\0", data2, 3), 0);
	UT_ASSERTeq(memcmp(data2 + 3, bufFF, sizeof(data2) - 3), 0);

	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 9);
	PMEMFILE_LSEEK(pfp, f, 100, SEEK_END, 9 + 100);
	PMEMFILE_WRITE(pfp, f, "XYZ\0", 4, 4);
	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 9 + 100 + 4);
	PMEMFILE_LSEEK(pfp, f, 0, SEEK_SET, 0);

	memset(data2, 0xff, sizeof(data2));
	PMEMFILE_READ(pfp, f, data2, sizeof(data2), 9 + 100 + 4);
	UT_ASSERTeq(memcmp("pmemin S\0", data2, 9), 0);
	UT_ASSERTeq(memcmp(data2 + 9, buf00, 100), 0);
	UT_ASSERTeq(memcmp("XYZ\0", data2 + 9 + 100, 4), 0);
	UT_ASSERTeq(memcmp(data2 + 9 + 100 + 4, bufFF,
			sizeof(data2) - 9 - 100 - 4), 0);

	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 9 + 100 + 4);

	PMEMFILE_CLOSE(pfp, f);


	_pmemfile_list_root(pfp, "/file1 9+100+4=113");

	_pmemfile_stats(pfp);

	PMEMFILE_UNLINK(pfp, "/file1");

	_pmemfile_stats(pfp);


	f = PMEMFILE_OPEN(pfp, "/file1", O_CREAT | O_EXCL | O_RDWR, 0644);

	PMEMFILE_WRITE(pfp, f, buf00, 4096, 4096);
	PMEMFILE_FILE_SIZE(pfp, f, 4096);

	PMEMFILE_WRITE(pfp, f, bufFF, 4096, 4096);
	PMEMFILE_FILE_SIZE(pfp, f, 8192);

	PMEMFILE_LSEEK(pfp, f, 0, SEEK_CUR, 8192);
	PMEMFILE_LSEEK(pfp, f, 4096, SEEK_SET, 4096);
	PMEMFILE_FILE_SIZE(pfp, f, 8192);

	PMEMFILE_READ(pfp, f, data2, 4096, 4096);
	PMEMFILE_FILE_SIZE(pfp, f, 8192);

	PMEMFILE_CLOSE(pfp, f);

	_pmemfile_list_root(pfp, "/file1 8192");
	_pmemfile_stats(pfp);

	PMEMFILE_UNLINK(pfp, "/file1");
}

static void
test2(PMEMfilepool *pfp)
{
	char buf00[128], bufFF[128], bufd[4096 * 4], buftmp[4096 * 4];

	memset(buf00, 0x00, sizeof(buf00));
	memset(bufFF, 0xFF, sizeof(bufFF));

	for (int i = 0; i < sizeof(bufd); ++i)
		bufd[i] = rand() % 255;

	PMEMfile *f = PMEMFILE_OPEN(pfp, "/file1", O_CREAT | O_EXCL | O_WRONLY,
			0644);

#define LEN (sizeof(bufd) - 1000)
#define LOOPS ((800 * 1024 * 1024) / LEN)
	for (int i = 0; i < LOOPS; ++i)
		PMEMFILE_WRITE(pfp, f, bufd, LEN, LEN);

	PMEMFILE_CLOSE(pfp, f);
	_pmemfile_list_root(pfp, "/file1 ~800MB");
	_pmemfile_stats(pfp);

	f = PMEMFILE_OPEN(pfp, "/file1", O_RDONLY);

	for (int i = 0; i < LOOPS; ++i) {
		memset(buftmp, 0, sizeof(buftmp));
		PMEMFILE_READ(pfp, f, buftmp, LEN, LEN);
		if (memcmp(buftmp, bufd, LEN) != 0)
			UT_ASSERT(0);
	}
#undef LEN
	PMEMFILE_READ(pfp, f, buftmp, 1023, 0);

	PMEMFILE_CLOSE(pfp, f);

	PMEMFILE_UNLINK(pfp, "/file1");
}

static void
test_trunc(PMEMfilepool *pfp)
{
	char bufFF[128], bufDD[128], buftmp[128];

	memset(bufFF, 0xFF, sizeof(bufFF));
	memset(bufDD, 0xDD, sizeof(bufDD));

	PMEMfile *f1 = PMEMFILE_OPEN(pfp, "/file1", O_CREAT | O_EXCL | O_WRONLY,
			0644);
	PMEMfile *f2 = PMEMFILE_OPEN(pfp, "/file2", O_CREAT | O_EXCL | O_WRONLY,
			0644);

	for (int i = 0; i < 100; ++i) {
		PMEMFILE_WRITE(pfp, f1, bufFF, 128, 128);
		PMEMFILE_WRITE(pfp, f1, bufDD, 128, 128);

		PMEMFILE_WRITE(pfp, f2, bufFF, 128, 128);
		PMEMFILE_WRITE(pfp, f2, bufDD, 128, 128);
	}

	PMEMFILE_CLOSE(pfp, f1);
	PMEMFILE_CLOSE(pfp, f2);
	_pmemfile_list_root(pfp, "/file1,file2 25600");
	_pmemfile_stats(pfp);

	f1 = PMEMFILE_OPEN(pfp, "/file1", O_RDWR | O_TRUNC, 0);

	f2 = PMEMFILE_OPEN(pfp, "/file2", O_RDWR | O_TRUNC, 0);

	PMEMFILE_READ(pfp, f1, buftmp, 128, 0);

	PMEMFILE_WRITE(pfp, f2, bufDD, 128, 128);

	PMEMFILE_CLOSE(pfp, f1);
	PMEMFILE_CLOSE(pfp, f2);

	_pmemfile_list_root(pfp, "/file1 0, /file2 128");
	_pmemfile_stats(pfp);

	PMEMFILE_UNLINK(pfp, "/file1");

	PMEMFILE_UNLINK(pfp, "/file2");
}

int
main(int argc, char *argv[])
{
	START(argc, argv, "file_core_rw");

	if (argc < 2)
		UT_FATAL("usage: %s file-name", argv[0]);

	const char *path = argv[1];

	PMEMfilepool *pfp = PMEMFILE_MKFS(path);

	_pmemfile_stats(pfp);

	_pmemfile_list_root(pfp, "no files");

	test1(pfp);
	_pmemfile_list_root(pfp, "no files");
	_pmemfile_stats(pfp);

	test2(pfp);
	_pmemfile_list_root(pfp, "no files");
	_pmemfile_stats(pfp);

	test_trunc(pfp);
	_pmemfile_list_root(pfp, "no files");
	_pmemfile_stats(pfp);

	pmemfile_pool_close(pfp);

	DONE(NULL);
}
