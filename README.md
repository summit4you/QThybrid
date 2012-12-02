QThybrid
========

QT hybrid . You can use HTML5 write C++ Application 

follow htmapp project. And I use CMAKE to republic.

The Orign URL is : [htmapp](http://code.google.com/p/htmapp/)

Thanks to Robert Umbehant

Copyright
=========

Robert Umbehant
	htmapp@wheresjames.com
	http://www.wheresjames.com
	
	  Redistribution and use in source and binary forms, with or
	without modification, are permitted for commercial and
	non-commercial purposes, provided that the following
	conditions are met:

	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* The names of the developers or contributors may not be used to
	  endorse or promote products derived from this software without
	  specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
	CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
	EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	
//------------------------------------------------------------------
// jQuery
//------------------------------------------------------------------

http://jquery.org/

	Copyright (c) 2011 John Resig, http://jquery.com/

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//------------------------------------------------------------------
// d3
//------------------------------------------------------------------

http://mbostock.github.com/d3/

	Copyright (c) 2010, Michael Bostock
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	* The name Michael Bostock may not be used to endorse or promote products
	  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL MICHAEL BOSTOCK BE LIABLE FOR ANY DIRECT,
	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
	EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//------------------------------------------------------------------
// Quicksand
//------------------------------------------------------------------

http://razorjack.net/quicksand/

	Copyright ?2010 Jacek Galanciak and agilope, 
	released under both MIT and GPL version 2 license

//------------------------------------------------------------------
// Icons
//------------------------------------------------------------------

http://openiconlibrary.sourceforge.net/

	#########################
	#   Open Icon Library   #
	#########################
	Originally created:  10 Nov 2009
	Icon creators:	see LICENSES file
	Archive Creator:	Jeff Israel
	
	#########################
	# Description           #
	#########################
	Open Icon Library is an archive of icon files from various sources, 
	design to offer a single location for free icons. 
	
	
	#########################
	# Licensing             #
	#########################
	The icons in this package are under various free/open licenses 
	such as GPL, Creative Commons, etc.  All sources with 
	their respective licenses are documented in the LICENSES file, 
	as well as in their metadata. 

LICENSE
=======

	/*------------------------------------------------------------------
	// Copyright (c) 1997 - 2012
	// Robert Umbehant
	// htmapp@wheresjames.com
	// http://www.wheresjames.com
	//
	// Redistribution and use in source and binary forms, with or
	// without modification, are permitted for commercial and
	// non-commercial purposes, provided that the following
	// conditions are met:
	//
	// * Redistributions of source code must retain the above copyright
	//   notice, this list of conditions and the following disclaimer.
	// * The names of the developers or contributors may not be used to
	//   endorse or promote products derived from this software without
	//   specific prior written permission.
	//
	//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
	//   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
	//   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	//   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	//   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	//   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	//   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	//   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
	//   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	//----------------------------------------------------------------*/

中文说明
=======

本文参考 Robert Umbehant 的源代码，文章参见这里[英文](http://www.codeproject.com/Articles/336018/Building-C-Applications-with-HTML5), [中文](http://scriptogr.am/bee/post/building-c-applications-with-html5)。 采用CMake对源代码进行重构，对源代码进行适当修正以支持VC编译。已在VC2008编译通过。

用法如下：

- 使用html5开发ui，并嵌入C++代码，具体参见[原文](http://www.codeproject.com/Articles/336018/Building-C-Applications-with-HTML5)。
- 使用编译成功的cii辅助工具，把html5编译成C++代码。命令方式如下

	`cii -i htm -o htmcpp -f "static int _internal_run( TPropertyBag< str::t_string8 > &in, TPropertyBag< str::t_string8 > &out )"`

- 与库htmapp及frwk一同编译

代码库中提供了一个磁盘检测的例子。

模板引擎
========

当前加入CTemplate的支持，使得你可以用MVC模式开发QThybrid应用。*.csp用于后台代码， *.htm用于前台代码。示例如下

index.csp

	<?global

		int i = 100;
		
	?>

	<?c
		std::map<string, string> values;
		values["i"] = i;
		values["title"] = "hello world";

		out << csp_template("tpl/index.htm", values);
	?>

解释

- <?global 括住的代表全局变量

- <? 括住的代表后台代码

- std::map<string, string>用于向模板传值， 例子中我们有两个模板参数

- csp_template用于调用模板， 参数1为模板的路径，参数2为模板参数容器

index.htm遵循CTemplate约定, 请参见[这里](http://code.google.com/p/ctemplate/?redir=1)

	<!DOCTYPE html>
	<html>
	<head>
		<title>{{title}}</title>
		</head>
	<body>
		{{i}}
	</body>
	</html>


例子参见 example/templateapp 

