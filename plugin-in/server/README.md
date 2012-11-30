# 服务器版本 #

为了让QThybrid开发更灵活，不需要每次重复编译整个WebKit。开发服务器版本。只需要通过

cii生成html5对应的代码，然后把代码编译成为 htmapp_resources.dll,放入服务器程序

同一个目录下，那么即可以发布程序

其中htmapp_resources需要修改为导出，_htmapp_resources全局变量

	extern "C" __declspec(dllexport)	SHmResourceInfo _htmapp_resources[];

enjoy QThybrid :)