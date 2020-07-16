#include <jni.h>
#include <android/log.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "mycommom.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <dlfcn.h>
#include <sys/mman.h>

void native_attachContextBaseContext(JNIEnv *env, jclass clazz,jobject ctx);
void native_onCreate(JNIEnv *env, jclass clazz);
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved);

#define  TAG    "info"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#define JNIREG_CLASS "com/example/jnishell3/StubApplication"
//#define JNIREG_CLASS "com/ali/mobisecenhance/StubApplication"


jclass Build_version;
jint sdk_int;
jclass ActivityThread;
jfieldID mPackages;
jclass myArrayMap;
jmethodID ArrayMap_get;
jfieldID  mBoundApplication;
jfieldID  mInitialApplication;
jfieldID  mAllApplications;
jmethodID currentActivityThread;

jclass AppBindData;
jfieldID  AppBindData_info;

jclass myArrayList;
jmethodID arraylist_size;
jmethodID arraylist_get;
jmethodID arraylist_set;


jclass myContext;
jmethodID context_getPackageName ;
jmethodID context_getApplicationInfo;
jmethodID context_getClassLoader;
jmethodID context_getAssets;
jmethodID context_getPackageResourePath;

jclass myWeakReference;
jmethodID WeakReference_get;

jclass myLoadedApk;
jfieldID  LoadedApk_mClassLoader;
jfieldID  LoadedApk_mApplication ;

jclass myApplicationInfo;
jfieldID  ApplicationInfo_dataDir;
jfieldID  ApplicationInfo_nativeLibraryDir;
jfieldID  ApplicationInfo_sourceDir ;

jclass myApplication;
jmethodID Application_onCreate;
jmethodID Application_attach;

jclass myContextWrapper;
jmethodID ContextWrapper_attachBaseContext ;

jclass myPathClassLoader;

jclass myBaseDexClassLoader;
jfieldID  BaseDexClassLoader_pathList ;

jclass myDexPathList;
jfieldID  DexPathList_dexElements;

jclass myElement;
jfieldID  DexPathList_Element_dexFile;
jfieldID  DexPathList_Element_file;

jclass myFile;
jmethodID myFile_getAbsolutePath;

jclass myDexFile;
jfieldID  mCookie;
jmethodID myOpenDexFile;

jclass myClassLoader;
jmethodID classLoader_findClass;

jclass mySystem;


jmethodID system_getProperty;
jclass status;

jmethodID SystemProperties_get;
jclass mySystemProperties;

int isDalvik;
int isArt;

jobject onCreateObj;
const char* mPackageName;
const char* mAbsolutePath_str;

JNINativeMethod *dvm_dalvik_system_DexFile;
void (*openDexFile)(const u4* args,union  JValue* pResult);
typedef int (*pArtFun)(int,int );

int dvm_Cookie=0;//4.4
jlong art_Cookie=0;//5.0-5.1
jobject art_MarCookie=0;//6.0

// 获取指定名称类的全局引用
jclass myFindClass(JNIEnv *env,jclass* ptr,char* name)
{
    jobject g_cls_string;
    jclass clazz = (*env)->FindClass(env,name);
    if(clazz)
    {
        //// 获取类的全局引用(class)
        g_cls_string = (*env)->NewGlobalRef(env,clazz);
        //将全局引用(class)存到ptr中
        *ptr=g_cls_string;
        return g_cls_string;
    }
    else
    {
        return 0;
    }

}

void init_class(JNIEnv *env, jobject obj,jobject ctx)
{
    // 获取类android.os.Build.VERSION的全局引用(class)
    if( !myFindClass(env,&Build_version,"android/os/Build$VERSION"))
    {
       // LOGI("ERROR:Build$VERSION");
        return;
    }
    //通过Build_VERSION获取当前SDK版本的ID，并通过ID调用获取当前的SDK版本
    jfieldID fieldID= ((*env)->GetStaticFieldID)(env, Build_version, "SDK_INT", "I");
    sdk_int=(*env)->GetStaticIntField(env,Build_version,fieldID);
    //LOGI("sdk_int %d\n",sdk_int);


// 获取类android.app.ActivityThread的全局引用
    if( !myFindClass(env, &ActivityThread, "android/app/ActivityThread"))
    {
        //LOGI("ERROR:ActivityThread");
        return;
    }
//判断sdk版本是否＞18，似乎必须大于18才行
    if(sdk_int>18)
    {
        //获取类android.app.ActivityThread的非静态成员变量mPackages的ID
        mPackages=(*env)->GetFieldID(env, ActivityThread, "mPackages",  "Landroid/util/ArrayMap;");
        // 获取类android.util.ArrayMap的全局引用(class)
        if ( !myFindClass(env, &myArrayMap, "android/util/ArrayMap") )
        {
            //LOGI("ERROR:myArrayMap");
            return;
        }
        //通过myArrayMap获取其get方法的ID
        ArrayMap_get=(*env)->GetMethodID(env,myArrayMap,"get","(Ljava/lang/Object;)Ljava/lang/Object;");

        //通过ActivityThread获取mBoundApplication的ID
        mBoundApplication = (*env)->GetFieldID(env,
                                               ActivityThread,
                                               "mBoundApplication",
                                               "Landroid/app/ActivityThread$AppBindData;");
        //通过ActivityThread获取mInitialApplication的ID
        mInitialApplication = (*env)->GetFieldID(
                env,
                ActivityThread,
                "mInitialApplication",
                "Landroid/app/Application;");
        //通过ActivityThread获取mAllApplications的ID
        mAllApplications = (*env)->GetFieldID(env,
                                              ActivityThread,
                                              "mAllApplications",
                                              "Ljava/util/ArrayList;");
        //通过ActivityThread获取currentActivityThread的ID
        currentActivityThread = (*env)->GetStaticMethodID(
                env,
                ActivityThread,
                "currentActivityThread",
                "()Landroid/app/ActivityThread;");
        //LOGI("ActivityThread:%p,%p,%p,%p",mBoundApplication,mInitialApplication,mAllApplications,currentActivityThread);


        // 获取类android.app.ActivityThread$AppBindData的全局引用（内部class）
        if ( !myFindClass(env, &AppBindData, "android/app/ActivityThread$AppBindData") )
        {
            //LOGI("ERROR:AppBindData");
            return;
        }
        //通过AppBindData获取info的ID
        AppBindData_info=(*env)->GetFieldID(env, AppBindData, "info", "Landroid/app/LoadedApk;");

        // 获取类java.util.ArrayList的全局引用（class）
        if ( !myFindClass(env, &myArrayList, "java/util/ArrayList") )
        {
            //LOGI("ERROR:myArrayList");
            return;
        }
        //通过ArrayList获取ArrayList的size、get、set方法的ID
        arraylist_size = (*env)->GetMethodID(env, myArrayList, "size", "()I");
        arraylist_get = (*env)->GetMethodID(env, myArrayList, "get", "(I)Ljava/lang/Object;");
        arraylist_set = (*env)->GetMethodID(env, myArrayList, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");

        // 获取类android.content.Context的全局引用（class）
        if ( !myFindClass(env, &myContext, "android/content/Context") )
        {
            //LOGI("ERROR:myContext");
            return;
        }
        //通过Context获取Context的getPackName、getApplicationInfo、getClassLoader、getAssets、getPackageResourePath方法的ID
        context_getPackageName = (*env)->GetMethodID(env, myContext, "getPackageName", "()Ljava/lang/String;");
        context_getApplicationInfo = (*env)->GetMethodID(
                env,
                myContext,
                "getApplicationInfo",
                "()Landroid/content/pm/ApplicationInfo;");
        context_getClassLoader = (*env)->GetMethodID(
                env,
                myContext,
                "getClassLoader",
                "()Ljava/lang/ClassLoader;");
        context_getAssets = (*env)->GetMethodID(
                env,
                myContext,
                "getAssets",
                "()Landroid/content/res/AssetManager;");
        context_getPackageResourePath = (*env)->GetMethodID(
                env,
                myContext,
                "getPackageResourcePath",
                "()Ljava/lang/String;");


        // 获取类java.lang.rel.WeakReference的全局引用（class）
        if ( !myFindClass(env, &myWeakReference, "java/lang/ref/WeakReference") )
        {
            //LOGI("ERROR:myWeakReference");
            return;
        }
        //通过WeakReference获取其的get方法的ID
        WeakReference_get = (*env)->GetMethodID(env, myWeakReference, "get", "()Ljava/lang/Object;");

        // 获取类android.app.LoadedApk的全局引用（class）
        if ( !myFindClass(env, &myLoadedApk, "android/app/LoadedApk") )
        {
            //LOGI("ERROR:myLoadedApk");
            return;
        }
        //通过LoadedApk获取其的mClassLoader、mApplication的ID
        LoadedApk_mClassLoader = (*env)->GetFieldID(
                env,
                myLoadedApk,
                "mClassLoader",
                "Ljava/lang/ClassLoader;");
        LoadedApk_mApplication = (*env)->GetFieldID(
                env,
                myLoadedApk,
                "mApplication",
                "Landroid/app/Application;");

        // 获取类android.content.pm.ApplicationInfo的全局引用（class）
        if ( !myFindClass(env, &myApplicationInfo, "android/content/pm/ApplicationInfo") )
        {
            //LOGI("ERROR:myApplicationInfo");
            return;
        }
        //通过ApplicationInfo获取其dataDir、nativeLibraryDir、sourceDir的ID
        ApplicationInfo_dataDir = (*env)->GetFieldID(
                env,
                myApplicationInfo,
                "dataDir",
                "Ljava/lang/String;");
        ApplicationInfo_nativeLibraryDir =(*env)->GetFieldID(
                env,
                myApplicationInfo,
                "nativeLibraryDir",
                "Ljava/lang/String;");
        ApplicationInfo_sourceDir = (*env)->GetFieldID(
                env,
                myApplicationInfo,
                "sourceDir",
                "Ljava/lang/String;");


        // 获取类android.app.Application的全局引用（class）
        if ( !myFindClass(env, &myApplication, "android/app/Application") )
        {
            //LOGI("ERROR:myApplication");
            return;
        }
        //通过Application类获取其onCreate、attach方法的ID
        Application_onCreate = (*env)->GetMethodID(env, myApplication, "onCreate", "()V");
        Application_attach = (*env)->GetMethodID(
                env,
                myApplication,
                "attach",
                "(Landroid/content/Context;)V");
        /*
		StubApplication-->atachContextBaseContext()-->super.attachBaseContext
		*/
        // 获取类android.content.ContextWrapper的全局引用（class）
        if ( !myFindClass(env, &myContextWrapper, "android/content/ContextWrapper") )
        {
            //LOGI("ERROR:myContextWrapper");
            return;
        }
        //通过android.content.ContextWrapper类获取其attachBaseContext方法的ID
        ContextWrapper_attachBaseContext = (*env)->GetMethodID(
                env,
                myContextWrapper,
                "attachBaseContext",
                "(Landroid/content/Context;)V");



        //LOGI("PathClassLoader start");
        // 获取类dalvik.system.PathClassLoader的全局引用（class）
        if ( !myFindClass(env, &myPathClassLoader, "dalvik/system/PathClassLoader") )
        {
            //LOGI("ERROR:myPathClassLoader");
            return;
        }
        //判断sdk版本
        if(sdk_int>13)
        {
            // 获取类dalvik.system.BaseDexClassLoader的全局引用（class）
            if ( !myFindClass(env, &myBaseDexClassLoader, "dalvik/system/BaseDexClassLoader") )
            {
                //LOGI("ERROR:myBaseDexClassLoader");
                return;
            }
            //通过BaseDexClassLoader类获取其pathlist的ID
            BaseDexClassLoader_pathList = (*env)->GetFieldID(
                    env,
                    myBaseDexClassLoader,
                    "pathList",
                    "Ldalvik/system/DexPathList;");
            // 获取类dalvik.system.DexPathList的全局引用（class）
            if ( !myFindClass(env, &myDexPathList, "dalvik/system/DexPathList") )
            {
                //LOGI("ERROR:myDexPathList");
                return;
            }
            //通过DexPathList类获取其pathlist的ID
            DexPathList_dexElements = (*env)->GetFieldID(
                    env,
                    myDexPathList,
                    "dexElements",
                    "[Ldalvik/system/DexPathList$Element;");
            // 获取类dalvik.system.DexPathList.Element的全局引用（内部class）
            if ( !myFindClass(env, &myElement, "dalvik/system/DexPathList$Element") )
            {
                //LOGI("ERROR:myElement");
                return;
            }

            //通过Element类获取其dexFile的ID
            DexPathList_Element_dexFile = (*env)->GetFieldID(
                    env,
                    myElement,
                    "dexFile",
                    "Ldalvik/system/DexFile;");
            //判断sdk版本，这里是基于安卓6.0的初始化
            /*
            由于6.0和其之前的版本存在差异，需要反射其他代码
            */
            if(sdk_int>22){//6.0
                //通过Element类获取其dir的ID
                DexPathList_Element_file = (*env)->GetFieldID(env, myElement, "dir", "Ljava/io/File;");
            }else{
                //通过Element类获取其dir的ID
                DexPathList_Element_file = (*env)->GetFieldID(env, myElement, "file", "Ljava/io/File;");
            }
            //获取类java.io.File的全局引用（class）
            if ( !myFindClass(env, &myFile, "java/io/File") )
            {
                //LOGI("ERROR:myFile");
                return;
            }
            //通过myFile类获取其getAbsolutePath的ID
            myFile_getAbsolutePath = (*env)->GetMethodID(
                    env,
                    myFile,
                    "getAbsolutePath",
                    "()Ljava/lang/String;");
            //LOGI("PathClassLoader end");
            //获取类dalvik.system.DexFile的全局引用（class）
            if ( !myFindClass(env, &myDexFile, "dalvik/system/DexFile") )
            {
                //LOGI("ERROR:myDexFile");
                return;
            }
            //判断sdk版本（其实这里有点累赘了可以把代码放在前面的判断）
            if(sdk_int>22)
            {//通过DexFile类获取其mCookie的ID和静态方法openDexFile的ID
                mCookie = (*env)->GetFieldID(env, myDexFile, "mCookie", "Ljava/lang/Object;");
                myOpenDexFile=(*env)->GetStaticMethodID(env, myDexFile, "openDexFile", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/Object;");
            }

            else if ( sdk_int > 19 )//5.0版本的mCookie和myOpenDexFile的签名和6.0存在差异
            {//通过DexFile类获取其mCookie的ID和静态方法openDexFile的ID
                mCookie = (*env)->GetFieldID(env, myDexFile, "mCookie", "J");

                myOpenDexFile=(*env)->GetStaticMethodID(env, myDexFile, "openDexFile", "(Ljava/lang/String;Ljava/lang/String;I)J");

            }

            else
            {
                //5.0一下版本的mCookie和myOpenDexFile的签名和6.0存在差异
                mCookie = (*env)->GetFieldID(env,myDexFile, "mCookie", "I");
                myOpenDexFile=(*env)->GetStaticMethodID(env, myDexFile, "openDexFile", "(Ljava/lang/String;Ljava/lang/String;I)I");

            }

            if ( !myFindClass(env, &myClassLoader, "java/lang/ClassLoader") )
            {
                //LOGI("ERROR:myClassLoader");
                return;
            }
            //android 5+以上无法用findClass找到android.app.Application类
            classLoader_findClass = (*env)->GetMethodID( env,
                                                         myClassLoader,
                                                         "loadClass",
                                                         "(Ljava/lang/String;)Ljava/lang/Class;");

            //LOGI("System start");
            //获取类java.lang.System的全局引用（class）
            if ( !myFindClass(env, &mySystem, "java/lang/System") )
            {
                //LOGI("ERROR:myClassLoader");
                return;
            }
            //通过System类获取getProperty静态方法的ID
            system_getProperty = (*env)->GetStaticMethodID(
                    env,
                    mySystem,
                    "getProperty",
                    "(Ljava/lang/String;)Ljava/lang/String;");

            //LOGI("SystemProperties start");

            status= myFindClass(env,
                                &mySystemProperties,
                                "android/os/SystemProperties");

            if(status)
            {
                /*            SystemProperties_get = (*env)->GetStaticMethodID(
                                    env,
                                    mySystemProperties,
                                    "get",
                                    "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");*/
                SystemProperties_get = (*env)->GetStaticMethodID(
                        env,
                        mySystemProperties,
                        "get",
                        "(Ljava/lang/String;)Ljava/lang/String;");
                //获取jstring类型的变量vmname，存放"java.vm.name"字符串
                jstring vmname = (*env)->NewStringUTF(env, "java.vm.name");
                //通过获取的getProperty方法的ID，调用有参方法getProperty("java.vm.name")，获取当前安卓虚拟机的名字
                jobject tmp= (*env)->CallStaticObjectMethod(env, mySystem, system_getProperty, vmname);

                // 将tmp转换成C语言字符串
                const char* v22 =  (*env)->GetStringUTFChars(env, tmp, 0);
                //LOGI("------- vmNameStr:%s", v22);//将转换成c语言字符的虚拟机名字打印
                (*env)->ReleaseStringUTFChars(env, tmp, v22);

                // persist.sys.dalvik.vm.lib
                // persist.sys.dalvik.vm.lib.2
                jstring vmlibstr = (*env)->NewStringUTF(env, "persist.sys.dalvik.vm.lib.2");

                //这里反编译出错
                jobject runtime = (*env)->CallStaticObjectMethod(env, mySystemProperties, SystemProperties_get, vmlibstr );

                const char* v28 = (*env)->GetStringUTFChars(env, runtime, 0);

                //释放内存空间
                (*env)->ReleaseStringUTFChars(env, runtime, v28);
                //获取jstring类型的变量vm_version，存放"java.vm.version"字符串
                jstring vm_version = (*env)->NewStringUTF(env, "java.vm.version");
                //通过获取的getProperty方法的ID，调用有参方法getProperty("java.vm.name")，获取当前安卓虚拟机的版本
                jobject v32 = (*env)->CallStaticObjectMethod(env, mySystem, system_getProperty, vm_version);
                // 将v32转换成C语言字符串
                const char* runtime_version = (*env)->GetStringUTFChars(env, v32, 0);
                //LOGI("---- vmVersionStr:%s", runtime_version);
                //将虚拟机版本字符串转化成双精度浮点型(double)
                double d=atof(runtime_version);
                //根据逻辑，如果d>2则不是Dalvik虚拟机
                if(d>2)
                    isDalvik=0;
                else
                    isDalvik=1;
                //释放内存
                (*env)->ReleaseStringUTFChars(env, v32, runtime_version);

                return ;
            }

        }
    }
}


//在JNINativeMethod结构体中根据函数名称和函数签名字符串匹配查找到该函数对应的函数调用指针
int lookup(JNINativeMethod *table, const char *name, const char *sig,void (**fnPtrout)(u4 const *, union JValue *))
{
    int i = 0;
    while (table[i].name != NULL)
    {
        //LOGI("lookup %d %s" ,i,table[i].name);
        // 根据函数名称和函数签名进行匹配查找
        if ((strcmp(name, table[i].name) == 0)
            && (strcmp(sig, table[i].signature) == 0))
        {	//获取该函数的名称和函数签名对应的指针
            *fnPtrout = table[i].fnPtr;
            return 1;
        }
        i++;
    }
    return 0;
}
/**
 *三个参数，env，application，szPath
 *env指针，application为apk当前的Application对象，szPath="/data/data/%s/files/dump.dex"字符串
 *将/data/data/<packagename>/asserts/dump.dex文件写入/data/data/<packagename>/files/dump.dex文件中
 *
 *
 */
void myExtractFile(JNIEnv* env,jobject application_obj,const char* szPath)
{
    //szPath=/data/data/%s/files/dump.dex
    AAssetManager* mgr;//声明资源管理器对象mgr
    //判断szPath路径下是否已经存在dump.dex文件
    if(access(szPath,R_OK))
    {
        //通过application对象获取application类
        jclass v19 = (*env)->GetObjectClass(env, application_obj);
        //通过application类获取getAssets()方法的ID
        jmethodID v20 = (*env)->GetMethodID(env, v19, "getAssets", "()Landroid/content/res/AssetManager;");
        //调用getAssets()方法返回值给v22(obj)
        jobject v22 = (*env)->CallObjectMethod(env, application_obj, v20);
        //给定一个Dalvik AssetManager对象，获取相应的本地AAssetManager对象。
        mgr= AAssetManager_fromJava(env, v22);
        if(mgr==NULL)
        { //获取失败
            //LOGI(" %s","AAssetManager==NULL");
            return ;
        }
        //打开apk资源文件夹下的dump.dex
        //即data/data/<packagename>/assets/dump.dex
        AAsset* asset = AAssetManager_open(mgr, "dump.dex",AASSET_MODE_STREAMING);
        FILE* file;
        void* buffer;
        int numBytesRead;
        if(asset)
        {
            //以szPath的字符串路径创建新的file(dump.dex)位于/data/data/<packagename>/files/目录下
            file=fopen(szPath,"w");
            //       int bufferSize = AAsset_getLength(asset);
            //       LOGI("buffersize is %d",bufferSize);
            //申请1024临时空间
            buffer=malloc(1024);
            while(1)
            { //将资源文件目录下的dump.dex文件读取出来放到申请的临时空间buffer中
                numBytesRead=AAsset_read(asset, buffer, 1024);
                //读取完成，结束while循环
                if(numBytesRead<=0)
                    break;
                //将存放到/data/data/<packagename>/files/file 文件中
                fwrite(buffer,numBytesRead,1,file);
            }
            free(buffer);
            fclose(file);
            AAsset_close(asset);
        }
        else
        {
            //LOGI("Error AAssetManager_open");
            return;
        }

    }
    else
    {
        //LOGI("dump.dex existed");
    }
}
/**
 *Dalvik模式下加载dex
 *将dump.dex映射到内存中，获取其返回值 *libdvm.so-->dvm_dalvik_system_DexFile(结构体)-->openDexFile(函数)-->Dalvik_dalvik_system_DexFile_openDexFile_by*tearray(const u4* arg,JValue*pResult)
 *优点：通过bytearry进行加载返回mCookie不会暴露解密完成后dex文件在内存中的位置
 */
void myLoadDex_dvm(JNIEnv* env,char* szPath)
{
    //加载动态库libdvm.so
    void *ldvm = (void*) dlopen("libdvm.so", 1);
    //获取动态库"libdvm.so"的导出结构体dvm_dalcik_system_DexFile,该结构体如下，里面存着各种方法：

/**
 // jni函数注册的结构体DalvikNativeMethod
 const DalvikNativeMethod dvm_dalvik_system_DexFile[] = {
    { "openDexFileNative",  "(Ljava/lang/String;Ljava/lang/String;I)I",
        Dalvik_dalvik_system_DexFile_openDexFileNative },
    { "openDexFile",        "([B)I",
        Dalvik_dalvik_system_DexFile_openDexFile_bytearray },
    { "closeDexFile",       "(I)V",
        Dalvik_dalvik_system_DexFile_closeDexFile },
    { "defineClassNative",  "(Ljava/lang/String;Ljava/lang/ClassLoader;I)Ljava/lang/Class;",
        Dalvik_dalvik_system_DexFile_defineClassNative },
    { "getClassNameList",   "(I)[Ljava/lang/String;",
        Dalvik_dalvik_system_DexFile_getClassNameList },
    { "isDexOptNeeded",     "(Ljava/lang/String;)Z",
        Dalvik_dalvik_system_DexFile_isDexOptNeeded },
    { NULL, NULL, NULL },
};
**/
    JNINativeMethod* dvm_dalvik_system_DexFile = (JNINativeMethod*) dlsym(ldvm,"dvm_dalvik_system_DexFile");
    //找到dvm_dalvik_system_DexFile结构体的openDexFile方法
    if(0 == lookup(dvm_dalvik_system_DexFile, "openDexFile", "([B)I", &openDexFile))
    {
        openDexFile = NULL;
        //LOGI("method does not found ");//如果没有找到，就返回
        return ;
    }
    else
    {
        //LOGI("openDexFile method found ! HAVE_BIG_ENDIAN");
    }
    int handle;
    struct stat buf={0};
    //打开szPath文件下的dex文件/data/data/<pacakgename>/files/dump.dex
    handle=open(szPath,0);
    //LOGI("handle:%X\n",handle);
    if(!handle)
    {//打开失败
        //LOGI("open dump.dex failed");
        return;
    }
    //获取dex文件的大小
    int status=fstat(handle,&buf);
    if(status)
    {
        //LOGI("fstat failed");
        return;
    }
    //获取dex文件长度

    int dexLen=buf.st_size;
    //LOGI("dexLen:%d,st_blksize:%d",dexLen,(int)buf.st_blksize);
    //#define PROT_READ 0x1     /* Page can be read.  */
    //#define PROT_WRITE  0x2     /* Page can be written.  */
    //#define PROT_EXEC 0x4     /* Page can be executed.  */
    //#define PROT_NONE 0x0     /* Page can not be accessed.  */

    //#define MAP_SHARED  0x01    /* Share changes.  */
    //#define MAP_PRIVATE 0x02    /* Changes are private.  */
    //将dex映射进入内存mmap函数
    //映射起始地址0，映射内存区域大小dexLen，期望的内存保护标志3，映射对象类型1，文件描述符handle(open函数返
    //回值),被映射对象从哪里开始对应0
    char* dexbase = (char*)mmap(0, dexLen, 3, 2, handle, 0);
    /*LOGI("dex magic %c %c %c %c %c %c %c",
         *dexbase,
         *(dexbase + 1),
         *(dexbase + 2),
         *(dexbase + 3),
         *(dexbase + 4),
         *(dexbase + 5),
         *(dexbase + 6));*/


    char* arr;
    //申请dexLen+16大小的空间Dalvik构建Dalvik_dalvik_system_DexFile_openDexFile_bytearray函数第一个传入参数args
    arr=(char*)malloc(16+dexLen);
    ArrayObject *ao=(ArrayObject*)arr;
    //LOGI("sizeof ArrayObject:%d",sizeof(ArrayObject));
    //ao获取dexLen
    ao->length=dexLen;
    //为什么获取的空间大小必须为dexLen+16呢？
    memcpy(arr+16,dexbase,dexLen);
    munmap(dexbase, dexLen);
    u4 args[] = { (u4) ao };
    union JValue pResult;
    jint result;
    if(openDexFile != NULL)
    {
        //调用Dalvik_dalvik_system_DexFile_openDexFile_bytearray加载dex文件，返回mCookie
        openDexFile(args,&pResult);
        result = (jint) pResult.l;
        dvm_Cookie=result;
        //LOGI("Dalvik Cookie :%X" , result);
    }
}
/**
 *art模式下加载dex
 *暂时没有考虑到暴露内存中dex文件的问题
 *
 */
void myLoadDex_art(JNIEnv* env,char* szPath)
{

    jstring inPath=(*env)->NewStringUTF(env,szPath);

    if(sdk_int>22)//6.0以上
    {
        art_MarCookie=(*env)->CallStaticObjectMethod(env, myDexFile, myOpenDexFile, inPath,0,0);
        //LOGI("----MarCookie:%p",art_MarCookie);
    }

    else
    {
        art_Cookie=(*env)->CallStaticLongMethod(env, myDexFile, myOpenDexFile, inPath,0,0);
        //LOGI("----artCookie:%llx",art_Cookie);
    }

    void* dlart=dlopen("libart.so",1);
    pArtFun pArtDexFileFindClassDef=(pArtFun)dlsym(dlart,"_ZNK3art7DexFile12FindClassDefEt");
    //LOGI("pArtDexFileFindClassDef:%p",pArtDexFileFindClassDef);



}

void replace_classloader_cookie(JNIEnv *env,jobject classLoader)
{
    if(sdk_int>13)
    {
        // "java/lang/ClassLoader"-->pathList对象
        jobject v28 = (*env)->GetObjectField(env, classLoader, BaseDexClassLoader_pathList);
        //获取Elements数组对象
        //pathList-->Element数组对象
        jobject v15 = (*env)->GetObjectField(env, v28, DexPathList_dexElements);
        int count = (*env)->GetArrayLength(env, v15);//获取Element数组长度
        //由于只有一个dex文件，count一直是1
        //LOGI("element count: %d", count);
        int i=0;
        while(i<count)
        {
            //获取Element数组的第i个元素
            jobject Element = (*env)->GetObjectArrayElement(env, v15, i);
            //获取dDexPathList&Element示例对象的非静态成员变量dir/file，这些好像有点多余的感觉
            jobject fileclazz= (*env)->GetObjectField(env, Element, DexPathList_Element_file);// 获取file类
            jobject v20 = (*env)->CallObjectMethod(env, fileclazz, myFile_getAbsolutePath);// file.getAbsolutePath()
            const char* str = (*env)->GetStringUTFChars(env, v20, 0);
            //android 6.0下str为:/
            //LOGI("element is %s", str);
            //
            /*int length = ((*env)->GetStringUTFLength)(env, v20);
            int cmpstatus = strncasecmp("apk", (length - 3 + str), 3);
            ((*env)->ReleaseStringUTFChars)(env, v20, str);*/
            //通过Element获取DexFile对象
            jobject DexFile = (*env)->GetObjectField(env, Element, DexPathList_Element_dexFile);// 获取DexFile类
            //判断sdk版本，并替换对应的mCookie
            if(sdk_int<=19)
            {
                //LOGI("SetIntField");
                //LOGI("---dvm_Cookie:%X",dvm_Cookie);
                (*env)->SetIntField(env, DexFile, mCookie, dvm_Cookie);
            }
            else if(sdk_int>22)
            {
                //LOGI("SetObjectField");
                //LOGI("---art_MarCookie:%X",art_MarCookie);
                (*env)->SetObjectField(env, DexFile, mCookie, art_MarCookie);
            }
            else
            {
                //LOGI("SetLongField");
                //LOGI("----artCookie:%llx",art_Cookie);
                (*env)->SetLongField(env, DexFile, mCookie, art_Cookie);
            }
            /*if(!cmpstatus)
            {

                break;
            }*/
            i++;
        }
        //LOGI("exit replace_classloader_cookie");
    }
}

void native_attachContextBaseContext(JNIEnv *env, jobject obj,jobject ctx)
{
    jobject application_obj=obj;
    // 通过反射调用，获取必要的函数调用id和成员变量获取id以备用以及当前Android虚拟机所处的运行模式
    init_class(env,obj,ctx);
    //LOGI("arg:application_obj:%p, myContextWrapper:%p, ContextWrapper_attachBaseContext:%p",application_obj, myContextWrapper, ContextWrapper_attachBaseContext);

    //这个地方ida反编译出错 少了参数ctx
    /**
     *通过init_class生成的ContextWrapper和获取的attachBaseContext
     *调用父类的方法android.content.ContextWrapper.attachBaseContext(ctx)
    */
    (*env)->CallNonvirtualVoidMethod(env, application_obj, myContextWrapper, ContextWrapper_attachBaseContext,ctx);


    /**
     *首先通过obj和env获取StubApplication类，再通过其获取到它的getFilesDir()方法
     *然后调用getFileDir()方法获取能得到文件路径的File实例对象
     *再调用File对象获取File类的getAbsolutePath()方法，获取到文件路径的对象，在将其转化成C语言字符串 *StubApplication-->getFilesDir()-->File(obj)-->File(class)-->getAbsolutePath()-->AbsolutePath(obj)-->Absolut*ePath(jstring)
     */
    jclass v12 = (*env)->GetObjectClass(env, application_obj);// 获取StubApplication类
    jmethodID v13 =(*env)->GetMethodID(env, v12, "getFilesDir", "()Ljava/io/File;");//获取getFilesDir()方法
    // 调用File.getFilesDir()方法，该方法返回/data/data/<packagename>/files的File实例对象
    jobject file_obj = (*env)->CallObjectMethod(env,obj , v13);
    //file_obj(obj)-->file_classz(class),通过File对象获取File类
    jclass file_classz=(*env)->GetObjectClass(env,file_obj);
    //获取file类的getAbsolutePath方法ID
    jmethodID v18 = (*env)->GetMethodID(env, file_classz, "getAbsolutePath", "()Ljava/lang/String;");
    //调用File.getAbsolutePath()获取文件路径/data/data/<packagename>/files
    jobject mAbsolutePath = (*env)->CallObjectMethod(env, file_obj, v18);

    //6.0下为/data/user/0/packagename/files/目录
    //将文件路径对象转化成字符串
    mAbsolutePath_str=(*env)->GetStringUTFChars(env,mAbsolutePath,0);
    //LOGI("global files path is %s",mAbsolutePath_str);

    //return ApplicationInfo
    /**
     *调用一些初始化的方法获取外壳apk的so文件路径，
     *首先根据当前application_obj调用context的getApplicationInfo方法获取ApplicationInfo实例对象
     *然后Application对象的nativeLibraryDir方法获取so文件路径的对象
     *最后将so文件路径对象转化成C语言字符串，存放到mNativeLibraryDir对象中 *context.getApplicationInfo()-->ApplicationInfo(obj)-->ApplicationInfo.nativeLibraryDir()-->v24(obj)-->mNati*veLibraryDir(const char)
     */
    //调用getApplicationInfo方法获取当前ApplicationInfo
    jobject ApplicationInfo = (*env)->CallObjectMethod(env, application_obj, context_getApplicationInfo);
    //获取so文件路径对象
    jobject v24 = (*env)->GetObjectField(env, ApplicationInfo, ApplicationInfo_nativeLibraryDir);
    //获取外壳apk的so文件路径
    const char* mNativeLibraryDir=(*env)->GetStringUTFChars(env,v24,0);
    //LOGI("mNativeLibraryDir is %s",mNativeLibraryDir);


    /**
     *调用一些初始化的方法ID获取apk的资源文件路径，然后指定外壳apk的进程APKAPATH作为资源存放路径
	 *调用StubApplication的getPackageResourcePath()获取资源文件路径对象，然后将其转化成char，在把它设置为APKPATH
	 *的值设置为apk的资源文件存放路径
	 *StubApplication.getPackageResourcePath()-->v32(obj)-->mPackageResourePath(char*)->setenv("APKPATH",char*,1)
     */
    //调用StuApplication的getPackageResourcePath()获取资源文件路径对象
    jobject v32 = (*env)->CallObjectMethod(env, application_obj, context_getPackageResourePath);
    //将资源文件路径对象转化成char*
    const char* mPackageResourePath=(*env)->GetStringUTFChars(env,v32,0);
    //将环境变量"APKPATH"的值设置为apk的资源文件存放路径
    setenv("APKPATH", mPackageResourePath, 1);
    //LOGI("APK Path is %s",mPackageResourePath);


    /**
     *调用Context的getPackageName方法获取APK包名
     *mPackageName接受包名字符串
     *
     */
    //调用Context的getPackageName()方法，获取APK包名对象
    jobject v36 =  (*env)->CallObjectMethod(env, ctx, context_getPackageName);
    //获取APK包名字符串(jstring)
    mPackageName=(*env)->GetStringUTFChars(env,v36,0);
    //LOGI("mPackageName:%s",mPackageName);

    /**
     *调用Context的getClassLoader()方法获取APK当前加载的ClassLoader
     *以便替换mCookie
     *
     */
    // public ClassLoader getClassLoader()
    //调用context的getClassLoader()方法，获取当前apk加载使用的ClassLoader实例
    jobject classLoader =(*env)->CallObjectMethod(env, ctx, context_getClassLoader);
    //LOGI("classLoader:%p",classLoader);

    /**
     *调用获取到的mPackgeName,从/data/data/<packagename>/files目录下释放出dump.dex
     */
    char szPath[260]={0};
    //   sprintf(szPath,"%s/dump.dex",mAbsolutePath_str);
    //使用获取/data/data/<packagename>/files/dump.dex的完整路径
    sprintf(szPath,"/data/data/%s/files/dump.dex",mPackageName);
    //LOGI("szPath:%s",szPath);
    /**
    *将/data/data/<packagename>/assets/dump.dex文件释放到内存中，在写入到
    * data/data/<pacakgename>/files/dump.dex文件中
    */
    myExtractFile(env,application_obj,szPath);

    //important
    //判断虚拟机类型，根据虚拟机的不同进行不同方式的dex文件加载
    if(isDalvik)
    {

        myLoadDex_dvm(env,szPath);
    }

    else
    {

        myLoadDex_art(env,szPath);
    }

    //important
    //用待加载的dex的ClassLoader的mCookie替换掉当前加载的ClassLoader的mCookie值
    replace_classloader_cookie(env,classLoader);

    //LOGI("enter new application");

/**
 * *(1)假设当前加载的dex文件继承了类android.app.Application,那么这里改为继承后的类的字符串，详细的查看androidmanife*st.xml *(2)假设当前加载的dex文件（真实的dex）没有继承实现android.app.Application,所以newapp会直接获取android.app.Applic*ation
 */
    jstring newapp= (*env)->NewStringUTF(env, "android.app.Application");
    //加载被保护dex文件的Application类
    jobject findclass_classz = (*env)->CallObjectMethod(env, classLoader,classLoader_findClass, newapp);
    if(!findclass_classz)
    {
        //LOGI("can't findClass realAppClass");
        return;
    }
    //通过获取的findclass获取Application类的构造函数<inti>的ID
    jmethodID initMethod = (*env)->GetMethodID(env, findclass_classz, "<init>", "()V");
    //通过findclass调用application类的构造函数创建application的实例对象
    onCreateObj = (*env)->NewObject(env, findclass_classz, initMethod);
    //调用attach方法
    (*env)->CallVoidMethod(env, onCreateObj, Application_attach, ctx);
    //创建待加载dex文件的application类对象的实例引用
    if ( onCreateObj )
    {
        onCreateObj = (*env)->NewGlobalRef(env, onCreateObj);
    }
    //LOGI("enter realAppClass");

}
/**
 *对应StubApplication的onCreate方法
 *目的：将壳apk的application类的实例对象替换成被源dex文件的application类实例对象(源dex文件一般是没有继承Applicat
 *ion,默认为android.app.application)，然后调用源dex文件的Application类对象实例的onCreate方法
 *
 *
 */
void native_onCreate(JNIEnv *env, jobject obj)
{
    //获取android.app.ActivityThread的静态方法currentActivityThread的对象
    //java代码：android.app.ActivityThread activityThread=ActivityThread.currentActivityThread();
    jobject theCurrentActivityThread = (*env)->CallStaticObjectMethod(env, ActivityThread, currentActivityThread);

    //final ArrayMap<String, WeakReference<LoadedApk>> mPackages= new ArrayMap<String, WeakReference<LoadedApk>>();
    /**
     *(1)获取壳apk的ActivityThread实例对象的的非静态成员变量mPackages
     *mPackages: ArrayMap<String,WeakReference<LoadedApk>> mPackages
     *再修改对应的LoadedApk实例对象的非静态成员变量mApplication为被源dex文件的类Application
     *java代码： android.util.ArrayMap arrayMap=activityThread.mPackages;
     *			 LoadedApk objLoadedApk=arrayMap.get("thePackagename");
     *
     */
    //获取android.util.ArrayMap的成员变量mPackages的对象
    jobject arraymap_mPackages =(*env)->GetObjectField(env, theCurrentActivityThread, mPackages);
    //获取壳apk包名字符串
    jstring thePackagename=(*env)->NewStringUTF(env,mPackageName);
    //LOGI("mPackageName %s",mPackageName);

    // 调用arrayMap_get函数 获取WeakReference<LoadedApk>类
    jobject v9 = (*env)->CallObjectMethod(env, arraymap_mPackages, ArrayMap_get, thePackagename);
    //调用WeakReference_get获取LoadedApk类
    jobject v15 = (*env)->CallObjectMethod(env, v9, WeakReference_get);
    //修改LoadedApk中的非静态成员变量mApplication为源dex的Application
    (*env)->SetObjectField(env, v15, LoadedApk_mApplication, onCreateObj);
    /**
     *(2)修改壳apk应用的ActivityThread实例对象的成员变量mInitialApplication
     *为源dex文件的Application对象实例
     *
     *
     */
    (*env)->SetObjectField(env, theCurrentActivityThread, mInitialApplication,onCreateObj);
    /**
 *(3)修改壳apk应用的ActivityThread实例对象的成员变量mBoundApplication中的LoadApk非静态成员变量info实例对象的
 *实例成员变量mApplication为源dex的Application对象
 *AppBindData mBoundApplication;
 *AppBindData{LoadedApk info;}
 *LoadedApk{Application mApplication}
 */
    // AppBindData mBoundApplication;
    jobject  v16 = (*env)->GetObjectField(env, theCurrentActivityThread, mBoundApplication);

    //AppBindData_info=(*env)->GetFieldID(env, AppBindData, "info", "Landroid/app/LoadedApk;");
    jobject v17 =(*env)->GetObjectField(env, v16, AppBindData_info);
    (*env)->SetObjectField(env, v17, LoadedApk_mApplication, onCreateObj);

    /*     mAllApplications = (*env)->GetFieldID(env,
                                  ActivityThread,
                                  "mAllApplications",
                                   "Ljava/util/ArrayList;");*/
    /**
     *(4)遍历壳apk应用的ActivityThread实例对象的非静态成员变量mAllApplications中的类Application实例对象
     *将壳apk的StubApplication替换为源dexapplication实例对象
     *StubApplication-->Application
     *ArrayList<Application> mAllApplications
     */
    //获取allApplications
    jobject v11 = (*env)->GetObjectField(env, theCurrentActivityThread, mAllApplications);
    //获取ArrayList的大小
    int count = (*env)->CallIntMethod(env, v11, arraylist_size);
    //LOGI("array_size:%d",count);
    int index = 0;
    while ( index < count )
    {
        //调用mAllApplication(ArrayList)的get方法获取每一个元素ArrayList.get(index)
        jobject v12 = (*env)->CallObjectMethod(env, v11, arraylist_get, index);
        //LOGI("compare: i=%d item=%p", index, v12);
        //在类对象mApplication实例中查找壳apk应用的StubApplication类对象
        if ( ((*env)->IsSameObject)(env, obj, v12) == 1 )//是否等于StubApplication类的对象
        {
            //LOGI("replace: find same replace");
            //调用ArrayList的set方法替换源Application的实例对象
            (*env)->CallObjectMethod(env, v11, arraylist_set, index,onCreateObj);
        }
        // _JNIEnv::DeleteLocalRef(env, v12);
        ++index;
    }
    //IDA反编译这里出错，多了一个参数
    (*env)->CallVoidMethod(env, onCreateObj, Application_onCreate);
    (*env)->DeleteGlobalRef(env, onCreateObj);

}


// 生成jni函数注册的jni函数table
static JNINativeMethod method_table[] = {
        // 被注册的函数的名称（java函数名称）、被注册函数的签名（javah获取）、
        // 被注册函数native实现的函数指针
        { "attachBaseContext", "(Landroid/content/Context;)V", (void*)native_attachContextBaseContext},
        { "onCreate","()V",(void*)native_onCreate},
};


static int registerNativeMethods(JNIEnv* env, const char* className,JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = (*env)->FindClass(env, className);
    if (clazz == 0) {
        return JNI_FALSE;
    }

    //LOGI("gMethods  %s,%s,%p\n ",gMethods[0].name,gMethods[0].signature,gMethods[0].fnPtr);

    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

int register_ndk_load(JNIEnv *env)
{
    // 对类"com/example/unpack/StubApplication"的函数attachBaseContext和onCreate进行注册
    return registerNativeMethods(env, JNIREG_CLASS,
                                 method_table, NELEM(method_table));
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{

JNIEnv* env = 0;
jint result = -1;
// LOGI("JNI_OnLoad is called");
if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
return result;
}

int status=register_ndk_load(env);
if(!status)
{
//LOGI("register call failed");
}

return JNI_VERSION_1_4;
}

