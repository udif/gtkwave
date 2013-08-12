/*
 * Copyright (c) 2013 Tony Bybell.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

public class fstScopeType
{
private fstScopeType() { }

public static final String [] FST_ST_NAMESTRINGS =
	{ "module", "task", "function", "begin", "fork", 
		"generate", "struct", "union", "class", "interface", 
		"package", "program" };

public static final int FST_ST_VCD_MIN = 0;
public static final int FST_ST_VCD_MODULE = 0;
public static final int FST_ST_VCD_TASK = 1;
public static final int FST_ST_VCD_FUNCTION = 2;
public static final int FST_ST_VCD_BEGIN = 3;
public static final int FST_ST_VCD_FORK = 4;
public static final int FST_ST_VCD_GENERATE = 5;
public static final int FST_ST_VCD_STRUCT = 6;
public static final int FST_ST_VCD_UNION = 7;
public static final int FST_ST_VCD_CLASS = 8;
public static final int FST_ST_VCD_INTERFACE = 9;
public static final int FST_ST_VCD_PACKAGE = 10;
public static final int FST_ST_VCD_PROGRAM = 11;
public static final int FST_ST_VCD_MAX = 11;

public static final int FST_ST_MAX = 11;

public static final int FST_ST_GEN_ATTRBEGIN = 252;
public static final int FST_ST_GEN_ATTREND = 253;

public static final int FST_ST_VCD_SCOPE = 254;
public static final int FST_ST_VCD_UPSCOPE = 255;
};

