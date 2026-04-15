#include <android_native_app_glue.h> // everything related to pure native activity we need this mainly android_main() and android_app struct
#include <android/log.h> // for __android_log_print()
#include <time.h>
#include <memory.h> // for memset()
#include <math.h>

typedef struct {
    struct android_app *app;
    bool bActive;
} Engine;
ANativeWindow *androidNativeWindow = NULL;

// global callback function declarations
void engine_handle_cmd(struct android_app *, int32_t);
int32_t engine_handle_input(struct android_app *, AInputEvent *);

// event handling related global variable
long touchStartTime = 0;
long pendingSingleTapTime = 0;

float touchStartX = 0.0f;
float touchStartY = 0.0f;
float lastTapX = 0.0f;
float lastTapY = 0.0f;

bool bTouchDown = false;
bool bDragging = false;
bool bLongPressDetected = false;
bool bDoubleTapDetected = false;
bool bPendingSingleTap = false;

void android_main(struct android_app *state)
{
    // code
    __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> started successfully !!!", __func__);

    // fullscreen and hide status bar
    JavaVM *vm = state->activity->vm;
    JNIEnv *env = NULL;

    vm->AttachCurrentThread(&env, NULL);
    jobject activityObject = state->activity->clazz;
    jclass activityClass = env->GetObjectClass(activityObject);
    jclass windowClass = env->FindClass("android/view/Window");
    jclass viewClass = env->FindClass("android/view/View");

    // get window method
    jmethodID getWindowMethod = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
    jobject windowObject = env->CallObjectMethod(activityObject, getWindowMethod);

    jmethodID getDecorViewMethod = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorViewObject = env->CallObjectMethod(windowObject, getDecorViewMethod);

    // get eight view class static fields
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_STABLE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_STABLE", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION", "I"));
    const int flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN", "I"));
    const int flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I"));
    const int flag_SYSTEM_UI_FLAG_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I"));
    const int flag_SYSTEM_UI_FLAG_LOW_PROFILE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LOW_PROFILE", "I"));
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I"));

    jmethodID setSystemUiVisibilityMethod = env->GetMethodID(viewClass, "setSystemUiVisibility", "(I)V");

    env->CallVoidMethod(
        decorViewObject, 
        setSystemUiVisibilityMethod, 
        flag_SYSTEM_UI_FLAG_IMMERSIVE |
        flag_SYSTEM_UI_FLAG_LAYOUT_STABLE |
        flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
        flag_SYSTEM_UI_FLAG_FULLSCREEN |
        flag_SYSTEM_UI_FLAG_LOW_PROFILE |
        flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY
    );

    // change to landscape mode
    jclass activityInfoClass = env->FindClass("android/content/pm/ActivityInfo");

    const int flag_SCREEN_ORIENTATION_LANDSCAPE = env->GetStaticIntField(activityInfoClass, env->GetStaticFieldID(activityInfoClass, "SCREEN_ORIENTATION_LANDSCAPE", "I"));

    jmethodID setRequestedOrientationMethod = env->GetMethodID(activityClass, "setRequestedOrientation", "(I)V");

    // now call method to set orientation
    env->CallVoidMethod(activityObject, setRequestedOrientationMethod, flag_SCREEN_ORIENTATION_LANDSCAPE);

    // detach vm from current thread
    vm->DetachCurrentThread();

    Engine engine;
    memset((void *)&engine, 0, sizeof(Engine));

    // initialize state
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    engine.app = state;

    while(1)
    {
        int identifyer = 0;
        struct android_poll_source *source = NULL;

        while((identifyer = ALooper_pollOnce(engine.bActive ? 0 : -1, NULL, NULL, (void **)&source)) >= 0)
        {
            // process system events
            if(source != NULL)
            {
                source->process(state, source);
            }

            // check when to exit
            if(state->destroyRequested != 0)
            {
                return;
            }
        }

        // handle long press
        if(bTouchDown == true && bDragging == false && bLongPressDetected == false && bDoubleTapDetected == false)
        {
            struct timespec ts;
            memset((void *)&ts, 0, sizeof(timespec));

            clock_gettime(CLOCK_MONOTONIC, &ts);

            long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

            if(now - touchStartTime > 500)
            {
                bLongPressDetected = true;
                __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> Long press detected", __func__);
            }
        }

        if(bPendingSingleTap == true && bDoubleTapDetected == false)
        {
            struct timespec ts;
            memset((void *)&ts, 0, sizeof(timespec));
            
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

            if(now - pendingSingleTapTime > 300)
            {
                bPendingSingleTap = false;
                __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> single tap detected", __func__);
            }
        }
    }
}

void engine_handle_cmd(struct android_app *app, int32_t cmd)
{
    Engine *engine = (Engine *)app->userData;

    switch (cmd)
    {
    case APP_CMD_SAVE_STATE:
        engine->bActive = false;
        break;
    
    case APP_CMD_INIT_WINDOW:
        if(engine->app->window != NULL)
        {
            engine->bActive = true;
            androidNativeWindow = engine->app->window;
            // draw backgroud color with pixel by pixel coloring by cpu
            ANativeWindow_Buffer buffer;
            uint32_t *pixels = NULL;
            uint32_t color;
            int x, y;

            // set the buffer geometry and format
            ANativeWindow_setBuffersGeometry(androidNativeWindow, 0, 0, WINDOW_FORMAT_RGBA_8888);

            if(ANativeWindow_lock(androidNativeWindow, &buffer, NULL) == 0)
            {
                pixels = (uint32_t *)buffer.bits;
                color = 0xFFFF00FF;

                for(y = 0; y < buffer.height; y++)
                {
                    for(x = 0; x < buffer.width; x++)
                    {
                        pixels[y * buffer.stride + x] = color;
                    }
                }
                ANativeWindow_unlockAndPost(androidNativeWindow);
            }
            __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> window is created", __func__);
        }
        else
        {
            androidNativeWindow = NULL;
        }
        break;
    
    case APP_CMD_TERM_WINDOW:
        __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> window is destroyed", __func__);
        break;

    case APP_CMD_GAINED_FOCUS:
        __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> window has got focus", __func__);
        engine->bActive = true;
        break;
    
    case APP_CMD_LOST_FOCUS:
        __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> window has lost focus", __func__);
        engine->bActive = false;
        break;
    default:
        break;
    }
}

int32_t engine_handle_input(struct android_app *app, AInputEvent *event)
{
    Engine *engine = (Engine *)app->userData;
    int32_t eventType = AInputEvent_getType(event);

    switch(eventType) 
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
            switch(action)
            {
                case AMOTION_EVENT_ACTION_DOWN:
                {
                    touchStartX = AMotionEvent_getX(event, 0);
                    touchStartY = AMotionEvent_getY(event, 0);

                    struct timespec ts;
                    memset((void *)&ts, 0, sizeof(timespec));

                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    touchStartTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

                    bTouchDown = true;
                    bDragging = false;
                    bLongPressDetected = false;
                    bDoubleTapDetected = false;

                    // check whether the down action is for second tab of the double tab
                    if(bPendingSingleTap == true)
                    {
                        long timeSinceLastTap = touchStartTime - pendingSingleTapTime;
                        float dX = touchStartX - lastTapX;
                        float dY = touchStartY - lastTapY;

                        float distance = sqrtf(dX * dX + dY * dY);
                        
                        if(timeSinceLastTap < 300 && distance < 100.0f)
                        {
                            __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> double tap detected", __func__);
                            bDoubleTapDetected = true;
                        }
                        else
                        {
                            // this is a new touch but not for double tap
                            bPendingSingleTap = false;
                        }
                    }
                }
                break;

                case AMOTION_EVENT_ACTION_MOVE:
                {
                    float currentX = AMotionEvent_getX(event, 0);
                    float currentY = AMotionEvent_getY(event, 0);

                    float dX = currentX - touchStartX;
                    float dY = currentY - touchStartY;

                    float distance = sqrtf(dX * dX + dY * dY);
                
                    if(distance > 50.0)
                    {
                        bDragging = true;
                    }
                }
                break;

                case AMOTION_EVENT_ACTION_UP:
                {
                    struct timespec ts;
                    memset((void *)&ts, 0, sizeof(timespec));

                    clock_gettime(CLOCK_MONOTONIC, &ts);

                    long currentTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

                    long duration = currentTime - touchStartTime;

                    float endX = AMotionEvent_getX(event, 0);
                    float endY = AMotionEvent_getY(event, 0);

                    float dX = endX - touchStartX;
                    float dY = endY - touchStartY;

                    float distance = sqrtf(dX * dX + dY * dY);

                    bTouchDown = false;

                    if(bDoubleTapDetected == true)
                    {
                        // this up belongs to second tap of double tap which we already handled
                    }
                    else if(bLongPressDetected == true)
                    {
                        // 
                    }
                    else if(bDragging == true && distance > 150.0f)
                    {
                        // swipe is detected
                        __android_log_print(ANDROID_LOG_INFO, "HGK:", "%s() -> swipe  detected", __func__);
                        ANativeActivity_finish(engine->app->activity);
                    }
                    else if(duration < 300.0f && distance < 50.0f)
                    {
                        // this means a short tap is detected and second tap may be pending this may be up or single tap which we handled in main loop
                        bPendingSingleTap = true;
                        pendingSingleTapTime = currentTime;
                        lastTapX = endX;
                        lastTapY = endY;
                    }
                    bDragging = false;
                }
                break;
            }
        }
        break;
    }
    return(0);
}
