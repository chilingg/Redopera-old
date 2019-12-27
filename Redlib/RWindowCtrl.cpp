#include "RWindowCtrl.h"
#include "RResource/RResource.h"
#include "RResource/RImage.h"

#include "RDebug.h"

bool RWindowCtrl::vSync_ = true;
GLFWwindow *RWindowCtrl::shareContext = nullptr;

RWindowCtrl::RWindowCtrl(const std::string &name, RController *parent):
    RController(name, parent),
    window_(nullptr),
    eventPool([](){})
{
    if(!shareContext)
    {
        //glfw错误回调
        glfwSetErrorCallback(glfwErrorCallback);
        //初始化GLFW
        if(!glfwInit())
        {
            printError("Failed to initialize GLFW");
            terminateFreeTree();
        }
        DefaultWindow();
        //加载手柄映射
        updateGamepadMappings(":/data/gamecontrollerdb.txt");
        //手柄连接回调
        glfwSetJoystickCallback(joystickPresentCallback);

        //需手动检测一次手柄连接，检测之前已连接的手柄
        for(int i = RInputModule::JOYSTICK_1; i <= RInputModule::JOYSTICK_LAST; ++i)
        {
            if(glfwJoystickIsGamepad(i))
                RInputModule::instance().addGamepad(RInputModule::toJoystickID(i));
        }
        //GLFW事件触发
        eventPool = &glfwPollEvents;
    }

    //一个线程窗口只能有一个上下文
    if(glfwGetCurrentContext())
    {
        printError("A thread can only have one context!");
        glfwTerminate();
        terminateFreeTree();
    }

    window_ = glfwCreateWindow(width_, height_, "Redopera", nullptr, shareContext);
    if(!window_)
    {
        printError("Fainled to create GLFW window!");
        glfwTerminate();
        terminateFreeTree();
    }
    if(!shareContext) shareContext = window_;

    //绑定上下文与this指针
    glfwSetWindowUserPointer(window_, this);
    //glfwGetWindowSize(window_, &width_, &height_);

    glfwMakeContextCurrent(window_);
    //初始化glad
    if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        printError("Failed to initialize GLAD");
        glfwTerminate();
        terminateFreeTree();
    }

#ifdef R_DEBUG
    RDebug() << printFormat::green << printFormat::bold << name << ": " << glGetString(GL_VERSION) << printFormat::non;
    //若启用OpenGL Debug
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        RDebug() << printFormat::green << printFormat::bold << "Enable OpenGL debug output" << printFormat::non;
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(openglDebugMessageCallback, nullptr);
        //过滤着色器编译成功消息通知
        glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_OTHER,
                              GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER,
                              GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    }
#endif

    //默认开启垂直同步
    glfwSwapInterval(1);
    //视口映射
    glViewport(0, 0, width_, height_);
    //设置混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //忽略不必要的Z轴片段
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);//小于或等于时通过
    //默认背景色
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
    //禁用字节对齐限制
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glfwSetFramebufferSizeCallback(window_, resizeCallback);
    //glfwSetKeyCallback(window_, keyboardCollback);
    //glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetScrollCallback(window_, mouseScrollCallback);
    glfwSetCursorPosCallback(window_, mouseMoveCallback);
    glfwSetWindowFocusCallback(window_, windowFocusCallback);
    glfwSetWindowCloseCallback(window_, windowCloseCallback);
    trackCursor();

    RImage img = RImage::getRedoperaIcon();
    GLFWimage icon{ img.width(), img.height(), img.data() };
    glfwSetWindowIcon(window_, 1, &icon);
}

RWindowCtrl::~RWindowCtrl()
{
    assert(window_);

    if(shareContext == window_)
    {
        glfwTerminate();
        shareContext = nullptr;
    }

    if(shareContext) glfwDestroyWindow(window_);
}

void RWindowCtrl::control()
{

}

void RWindowCtrl::translation(const RController::TranslationInfo &info)
{
    if(info.sender == this) //窗口默认只传递自己发送的变化信息
        RController::translation(info);
}

void RWindowCtrl::setWindowSize(int width, int height)
{
    width_ = width;
    height_ = height;
    glfwSetWindowSize(window_, width, height);
}

void RWindowCtrl::setWindowTitle(const std::string &title)
{
    glfwSetWindowTitle(window_, title.c_str());
}

void RWindowCtrl::setBackgroundColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

void RWindowCtrl::setBackgroundColor(RColor color)
{
    glClearColor(color.r() / 255.0f, color.g() / 255.0f, color.b() / 255.0f, color.a() / 255.0f);
}

void RWindowCtrl::setViewportRatio(double ratio)
{
    viewportRatio_ = ratio;
}

void RWindowCtrl::setViewportPattern(RWindowCtrl::ViewportPattern pattern)
{
    viewportPattern = pattern;
    glfwSetWindowSize(window_, width_, height_);
}

void RWindowCtrl::setVSync(bool enable)
{
    vSync_ = enable ? 1 : 0;//参数为1锁60帧
    glfwSwapInterval(vSync_);
}

void RWindowCtrl::setFullScreenWindow(bool b)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
    if(b)
    {
        glfwSetWindowMonitor(window_, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
    } else
    {
        glfwSetWindowMonitor(window_, nullptr, (vidmode->width-960)/2, (vidmode->height-540)/2,
                             960, 540, vidmode->refreshRate);
    }
}

void RWindowCtrl::setWindowMinimumSize(int minW, int minH)
{
#ifdef R_DEBUG
    if(minW < 1)
    {
        printError("Window size limit parameter error: minW = " + std::to_string(minW));
        minW = GLFW_DONT_CARE;
    }
    if(minH < 1)
    {
        printError("Window size limit parameter error: minH = " + std::to_string(minH));
        minH = GLFW_DONT_CARE;
    }
#endif

    glfwSetWindowSizeLimits(window_, minW, minH, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void RWindowCtrl::setWindowMaximumSize(int maxW, int maxH)
{
#ifdef R_DEBUG
    if(maxW < 1)
    {
        printError("Window size limit parameter error: maxW = " + std::to_string(maxW));
        maxW = GLFW_DONT_CARE;
    }
    if(maxH < 1)
    {
        printError("Window size limit parameter error: maxH = " + std::to_string(maxH));
        maxH = GLFW_DONT_CARE;
    }
#endif

    glfwSetWindowSizeLimits(window_, GLFW_DONT_CARE, GLFW_DONT_CARE, maxW, maxH);
}

void RWindowCtrl::setWindowSizeFixed(bool b)
{
    glfwSetWindowAttrib(window_, GLFW_RESIZABLE, b ? GLFW_FALSE : GLFW_TRUE);
}

void RWindowCtrl::setWindowDecrate(bool b)
{
    glfwSetWindowAttrib(window_, GLFW_DECORATED, b ? GLFW_TRUE: GLFW_FALSE);
}

void RWindowCtrl::setWindowFloatOnTop(bool b)
{
    glfwSetWindowAttrib(window_, GLFW_FLOATING, b ? GLFW_TRUE: GLFW_FALSE);
}

void RWindowCtrl::setWindowIcon(const RImage &img)
{
    GLFWimage icon{ img.width(), img.height(), const_cast<unsigned char*>(img.cdata()) };
    glfwSetWindowIcon(window_, 1, &icon);
}

void RWindowCtrl::setCursor()
{
    glfwSetCursor(window_, nullptr);
}

void RWindowCtrl::setCursor(RCursor &cursor)
{
    glfwSetCursor(window_, cursor.data());
}

double RWindowCtrl::getViewportRatio() const
{
    return viewportRatio_;
}

GLFWwindow *RWindowCtrl::getWindowHandle() const
{
    return window_;
}

bool RWindowCtrl::isFocus() const
{
    return focused_;
}

bool RWindowCtrl::isShouldCloused() const
{
    return glfwWindowShouldClose(window_);
}

void RWindowCtrl::DefaultWindow()
{
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//set主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//set副版本号
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//删除当前版本不推荐使用的功能
#ifdef R_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);//OpenGL的Debug输出
#endif
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);//创建窗口时初始不可见
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//use核心模式
    //glfwWindowHint(GLFW_DECORATED, enable ? GLFW_TRUE : GLFW_FALSE);//边框与标题栏
}

void RWindowCtrl::updateGamepadMappings(std::string path)
{
    std::string mappingCode = RResource::getTextFileContent(path);

    if(mappingCode.empty())
    {
        printError("Failed to update gamepad mapping! In path: " + path + '\n' +
                   "To https://github.com/gabomdq/SDL_GameControllerDB download gamecontrollerdb.txt file.");
        //加载内置的手柄映射
        mappingCode = std::string() + RInputModule::gamepadMappingCode0
                + RInputModule::gamepadMappingCode1 + RInputModule::gamepadMappingCode2;
    }

    glfwUpdateGamepadMappings(mappingCode.c_str());
}

void RWindowCtrl::closeWindow()
{
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

void RWindowCtrl::trackCursor()
{
    glfwSetCursorPosCallback(window_, mouseMoveCallback);
}

void RWindowCtrl::untrackCursor()
{
    glfwSetCursorPosCallback(window_, [](GLFWwindow *, double , double) {});
}

void RWindowCtrl::showWindow()
{
    glfwShowWindow(window_);
}

void RWindowCtrl::hideWindow()
{
    glfwHideWindow(window_);
}

int RWindowCtrl::exec()
{
    assert(!isLooped());
    RStartEvent initEvent(this);
    dispatchEvent(&initEvent);
    while(isLooped())
    {
        eventPool(); //更新事件

        if(focused_) //更新输入
        {
            //更新手柄输入
            RInputModule::instance().updateGamepad();
            //更新键鼠输入
            RInputModule::instance().updateKeyboardInput(window_);
            RInputModule::instance().updateMouseInput(window_);
            //发布输入事件
            RInputEvent e(this);
            dispatchEvent(&e);
        }

        //清屏 清除颜色缓冲和深度缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        allChildrenActive();
        control();

        glfwSwapBuffers(window_);
    }
    RFinishEvent closeEvent(this);
    dispatchEvent(&closeEvent);
    finished.emit();

    printError(isFailur(), "The Loop has unexpectedly finished");
    return status();
}

std::string RWindowCtrl::getDefaultName() const
{
    return "WindowCtrl";
}

void RWindowCtrl::startEvent(RStartEvent *)
{
#ifdef R_DEBUG
    printError(!glfwGetWindowAttrib(window_, GLFW_VISIBLE), name() + "Window is hide! in initialization Event");
#endif
}

void RWindowCtrl::finishEvent(RFinishEvent *)
{

}

RWindowCtrl *RWindowCtrl::getWindowUserCtrl(GLFWwindow *window)
{
    return static_cast<RWindowCtrl*>(glfwGetWindowUserPointer(window));
}

void RWindowCtrl::glfwErrorCallback(int error, const char *description)
{
    printError("Error " + std::to_string(error) + ": " + description);
}

void RWindowCtrl::openglDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                             GLsizei , const GLchar *message, const void *)
{
    std::string sourceStr;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "Application"; break;
    case GL_DEBUG_SOURCE_OTHER:
        sourceStr = "Other"; break;
    }

    std::string typeStr;
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "-Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "-Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "-Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "-Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "-Performance"; break;
    case GL_DEBUG_TYPE_MARKER:
        typeStr = "-Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        typeStr = "-Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:
        typeStr = "-Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:
        typeStr = "-Other"; break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cerr << '(' << id << ')' << sourceStr << typeStr << "-high " << ">> "
                  << message << std::endl;
        terminateFreeTree();
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cerr << '(' << id << ')' << sourceStr << typeStr << "-medium " << ">> "
                  << message << std::endl;
        //terminateFreeTree();
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << printFormat::yellow << printFormat::bold << '(' << id << ')' << sourceStr << typeStr << "-low "
                  << ">> " << message << printFormat::non << std::endl;
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << printFormat::green << printFormat::bold << '(' << id << ')' << sourceStr << typeStr << "-notification "
                  << ">> " << message << printFormat::non << std::endl;
        break;
    }
}

void RWindowCtrl::joystickPresentCallback(int jid, int event)
{
    if(event == RInputModule::JOYSTICK_CONNECTED && glfwJoystickIsGamepad(jid))//断开的JID无法通过
    {
        RInputModule::instance().addGamepad(RInputModule::toJoystickID(jid));
    }
    else if(event == RInputModule::JOYSTICK_DISCONNECTED)
    {
        RInputModule::instance().deleteGamepad(RInputModule::toJoystickID(jid));
    }
}

void RWindowCtrl::resizeCallback(GLFWwindow *window, int width, int height)
{
    RWindowCtrl *wctrl = getWindowUserCtrl(window);

    switch(wctrl->viewportPattern)
    {
    case FullWindow:
    {
        glViewport(0, 0, width, height);
        TranslationInfo info { wctrl, {width, height} };
        wctrl->translation(info);
        break;
    }
    case KeepScale:
    {
        double ratio = static_cast<double>(width) / height;
        int newW = width;
        int newH = height;
        if(ratio > wctrl->viewportRatio_)
        {
            newW = static_cast<int>(height * wctrl->viewportRatio_);
            glViewport((width - newW) / 2.0, 0, newW, newH);
        }
        else
        {
            newH = static_cast<int>(width / wctrl->viewportRatio_);
            glViewport(0, (height - newH) / 2.0, newW, newH);
        }
        wctrl->width_ = newW;
        wctrl->height_ = newH;
        TranslationInfo info { wctrl, {width, height} };
        wctrl->translation(info);
        break;
    }
    case FixedSize:
    {
        glViewport((width - wctrl->width_) / 2.0, (height - wctrl->height_) / 2.0, wctrl->width_, wctrl->height_);
        TranslationInfo info { wctrl, {width, height} };
        wctrl->translation(info);
        break;
    }
    }
}

void RWindowCtrl::mouseMoveCallback(GLFWwindow *, double xpos, double ypos)
{
    RInputModule::instance().updateCursorPos(static_cast<int>(xpos), static_cast<int>(ypos));
}

void RWindowCtrl::mouseScrollCallback(GLFWwindow *window, double , double y)
{
    RWindowCtrl *wctrl = getWindowUserCtrl(window);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    RScrollEvent e{wctrl, static_cast<int>(y), static_cast<int>(xpos), static_cast<int>(ypos)};
    wctrl->dispatchEvent(&e);
}

void RWindowCtrl::windowFocusCallback(GLFWwindow *window, int focused)
{
    RWindowCtrl *wctrl = getWindowUserCtrl(window);
    wctrl->focused_ = focused;
}

void RWindowCtrl::windowCloseCallback(GLFWwindow *window)
{
    RWindowCtrl *wctrl = getWindowUserCtrl(window);
    wctrl->breakLoop();

    if(wctrl->isLooped()) glfwSetWindowShouldClose(window, GLFW_FALSE);
}
