/* Based on examples/docs from:
 * http://java.sun.com/j2se/1.4.2/docs/guide/jni/spec/invocation.html#wp9502
 * http://java.sun.com/j2se/1.4.2/docs/guide/jni/spec/jniTOC.html
 * http://java.sun.com/docs/books/jni/html/invoke.html
 */
#include "sage3basic.h"
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "cmdline.h"
#include "commandline_processing.h"

#include "jserver.h"

// MH (5/9/2014): Update to use namespace X10
// DQ (10/14/2010):  This should only be included by source files that require it.
// This fixed a reported bug which caused conflicts with autoconf macros (e.g. PACKAGE_BUGREPORT).
// Interestingly it must be at the top of the list of include files.
#include "rose_config.h"

namespace Rose {
    namespace Frontend {
        namespace X10 {

            SgClassDefinition *x10LangPackageDefinition = NULL;
            SgClassType *ObjectClassType = NULL;
            SgClassType *StringClassType = NULL;
            SgClassType *ClassClassType = NULL;
            SgVariableSymbol *lengthSymbol = NULL;

            namespace X10c {
                using namespace std;

                SgSourceFile* X10c_globalFilePointer = 0;

                                std::list<std::string> classNames;

                typedef struct {
                   JavaVM * jvm;
                   JNIEnv * env;
                } JvmT;

                static JvmT je;

                //Warning ! do not make these static as gdb cannot stop at a static function
                JNIEnv*  get_env();
                JavaVM*  get_jvm();
                JvmT*    get_jvmEnv();
                void     jserver_start(JvmT* je);
                void     jserver_destroy();
                jclass   jserver_getJavaStringClass();

                jclass currentX10TraversalClass = NULL;
                JNIEnv *currentEnvironment = NULL;
                jmethodID mainMethod = NULL;
                jmethodID hasConflictsMethod = NULL;
                jmethodID getTempDirectoryMethod = NULL;
                jmethodID createTempFileMethod = NULL;
                jmethodID createTempNamedFileMethod = NULL;
                jmethodID createTempNamedDirectoryMethod = NULL;

                /* These  should be defined in jni.h */
                #ifndef JNI_VERSION_1_6
                #define JNI_VERSION_1_6 6
                #endif
                #ifndef JNI_VERSION_1_4
                #define JNI_VERSION_1_4 4
                #endif
                #ifndef JNI_VERSION_1_2
                #define JNI_VERSION_1_2 2
                #endif
                #ifndef JNI_VERSION_1_1
                #define JNI_VERSION_1_1 1
                #endif

                /* 
                 * This function does nothing since Java VM will
                 * be loaded and started automatically when it needed. 
                 *
                 */
                void jserver_init() {
                    return ;
                }
                
                JNIEnv* getEnv() {
                    return get_env();
                }
                
                void jserver_finish() {
                    return jserver_destroy();
                }

                jclass jserver_FindClass(const char *name) {
                    JNIEnv *env = get_env();
                    return env->FindClass(name);
                };

                jmethodID jserver_GetMethodID(int static_method, jclass obj_class, const char* name, const char* path) {
                    JNIEnv *env = get_env();
                    if (static_method==STATIC_METHOD)   
                        return  env->GetStaticMethodID(obj_class,name, path);
                    else
                        return  env->GetMethodID(obj_class,name, path);
                };

                void jserver_callMethod(jclass obj_class, jmethodID method, jobjectArray args) {  
                    JNIEnv *env = get_env();
                    env->CallStaticVoidMethod(obj_class, method, args);
                    return ;
                };

                int jserver_callBooleanMethod(jobject obj_class, jmethodID method) {
                    int retv = 0; 
                    JNIEnv *env = get_env();

                    retv = env->CallBooleanMethod(obj_class, method);
                    return retv;
                };


                //! Java VM loaded here
                /*! This function return <jvm, env> if Java VM is already load/run
                 *  otherwise load/run Java VM
                 */
                JvmT* get_jvmEnv() {
                    if (je.jvm == NULL ) {
                        jserver_start(&je);
                    }
                    return &je; 
                }

                JavaVM* get_jvm() {
                    return (get_jvmEnv())->jvm;
                }

                JNIEnv* get_env() {
                    return (get_jvmEnv())->env;
                }

                void jserver_start(JvmT* je) {
                    JavaVMInitArgs jvm_args;  /* VM initialization args.  */
                    // Pei-Hung (7/6/2017): Bumped to latest(?), unsure why 1_4 was used
                    jvm_args.version = JNI_VERSION_1_6;
                    jvm_args.ignoreUnrecognized = JNI_FALSE;

                    //----------------------------------------------------------------------------
                    // Add all our JVM options
                    //----------------------------------------------------------------------------
                    // MH (5/21/2014): X10 compiler options stored in the Cmdline::X10::x10c namespace.
                    std::list<std::string> jvm_options = Rose::Cmdline::X10::X10c::jvm_options;

                    std::string classpath = Rose::Cmdline::X10::X10c::GetRoseClasspath();
                    jvm_options.push_back(classpath);

                    // Pei-Hung (7/6/2017): Increasing stacksize fixes crashes on some rhel7 systems
                    std::string stack_option = "-Xss2m";
                    jvm_options.push_back(stack_option);

                    jvm_args.nOptions = jvm_options.size();
                    jvm_args.options = new JavaVMOption[jvm_args.nOptions];
                    for(int i=0; i < jvm_args.nOptions; ++i) {
                        std::string jvm_option = jvm_options.front();
                        if (Rose::Cmdline::verbose > 0)
                            std::cout << "[INFO] [x10c] jvm_option[" << i << "] = " << jvm_option << std::endl;
                        jvm_args.options[i].optionString = strdup(jvm_option.c_str());
                        jvm_options.pop_front();
                    }

                    //----------------------------------------------------------------------------
                    // Create and load the Java VM.
                    //----------------------------------------------------------------------------
                    int res = JNI_CreateJavaVM(&(je->jvm), (void **)&(je->env), &jvm_args);

                    if (res<0 || je->jvm==NULL || je->env==NULL) {
                        exit(1);
                    }

                    return ;
                }

                void jserver_destroy() {
                    JavaVM*  jvm = get_jvm();
                    if (jvm != NULL) {
                         /* Destroy the Java VM.  */
                         jvm->DestroyJavaVM();
                    }
                }

                void jserver_handleException() {
                    JNIEnv* env = get_env();
                    if(env->ExceptionOccurred())
                        env->ExceptionDescribe();

                    jserver_destroy();

                    /* Exit since the exception should mean we can't recover.  */
                    exit(1);
                }

                jobject jserver_getNewObject(jclass cls,jmethodID method, jobjectArray args, jstring name,jstring type) {
                    JNIEnv* env = get_env();
                    return env->NewObject(cls,method, args, name, type);
                }
  
                jstring jserver_getJavaString(const char *inString) {
                    JNIEnv*  env = get_env();
                    return env->NewStringUTF(inString);
                }

                jobjectArray jserver_getJavaStringArray(int argc, char **argv) {
                    jobjectArray argsStringArray = NULL;
                    jclass stringClass;
                    int i;
                    JNIEnv * env = get_env();

                    /* We need the String class because that is the underlying type of the array.  */
                    stringClass = jserver_getJavaStringClass();
                    if(stringClass == NULL)
                        return NULL;
                
                    /* Build a new object array.  Params are: env, length, class type of the array, initial object(?).  */
                    argsStringArray = env->NewObjectArray((argc-1), stringClass, NULL);
                
                    if(argsStringArray == NULL)
                        return NULL;
                
                    /* Put all args from argv, after the first (which is this program's
                           name) into the array of Strings for JavaMain.  The args array
                           for Java does not include the program name.  */
                    for(i = 1; i < argc; i++) {
                         env->SetObjectArrayElement(argsStringArray, (jsize)i-1, jserver_getJavaString(argv[i]));
                    }                
                    return argsStringArray;
                }

                jclass jserver_getJavaStringClass() {
                    JNIEnv * env = get_env();
                    return env->FindClass("java/lang/String");
                }
            }// Rose::Frontend::X10::X10c
        }// Rose::Frontend::X10
    }// Rose::Frontend
}// Rose

