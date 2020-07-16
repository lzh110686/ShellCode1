# ShellCode1
## 编译环境
Android Studio3.5
## 测试机器
Android5.0，和Android6.0部分机型测试通过
## 使用说明
编译一个源apk文件，将其原始dex文件在asset目录下，替换AndroidManifest.xml中的Activity为源apk的包名，同时修改native-lib.c文件的相关类名
更详细的可以参考我的另一篇文章https://bbs.pediy.com/thread-258264.htm，其中的第五点(5.自定义DexClassLoader)
