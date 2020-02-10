#include <RWindow.h>
#include <RDebug.h>
#include <RPlane.h>
#include <RTextbox.h>

using namespace Redopera;

class TestCtl: public RController
{
public:
    TestCtl(RController *parent, const std::string &name):
        RController(parent, name),
        plane(36, 36, RPoint(0, 0), RImage::redoperaIcon()),
        icon(16, 16, RPoint(0, 10), RImage::redoperaIcon()),
        arrow{ {"↑", 40, 40, 0, 0} },
        texts("Testing...", 70, 20, 10, 10)
    {
        plane.flipV();
        plane.setMargin(6);    // 外框一般用于检测碰撞
        plane.setPadding(2);   // 内框用于渲染尺寸
        plane.setAlign(RArea::Align::Mind, RArea::Align::Mind);
        plane.setMode(RArea::Mode::Auto);

        icon.flipV();
        icon.setAlign(RArea::Align::Mind, RArea::Align::Mind);

        texts.setFontSize(12);
        texts.setFontColor(50, 50, 70);     // 默认白色字体
        texts.setBackColor(0, 0, 0, 0);     // 默认透明，无需显示设置
        texts.setAlign(RArea::Align::Mind, RArea::Align::Left);  // 默认左上
        pro = RPlane::planeShader().getUniformLocation("projection");
        view = RPlane::planeShader().getUniformLocation("view");

        arrow[0].setAlign(RArea::Align::Left, RArea::Align::Mind);
        arrow[0].setFontSize(36);
        arrow[0].setFontColor(30, 30, 40);
        arrow[1] = arrow[0];
        arrow[2] = arrow[0];
        arrow[3] = arrow[0];

        arrow[1].rotateZ(glm::radians(90.0f));
        arrow[2].rotateZ(glm::radians(180.0f));
        arrow[3].rotateZ(glm::radians(270.0f));
    }

    void control() override
    {
        arrow[0].render();
        arrow[1].render();
        arrow[2].render();
        arrow[3].render();
        icon.rotateY(glfwGetTime() * 5);
        icon.render();
        texts.render();
        plane.render();

        // plane.edgingAll(); // 渲染全部边框
        // texts.edging(RColor(0x8000e0));  // 渲染边框
    }

protected:
    void startEvent(RStartEvent &) override
    {
        RWindow * window = RWindow::getMainWindow();

        plane.setPosX(window->width()/2 - plane.width()/2);
        plane.setPosY(window->height()/2 - plane.height()/2);

        // 必须设置一次的视口
        {
        // plane的着色器设置
        RShaderProgram::Interface inter = RPlane::planeShader().useInterface();
        inter.setViewprot(pro, 0, window->width(), 0, window->height());
        } // 一个线程同一时间内只能有一个Interface对象

        // textbox的着色器设置（与plane并不共享，建议永远与窗口尺寸等同，避免字体渲染虚化）
        const RShaderProgram &shaders = RTextsbxo::textboxShader();
        RShaderProgram::Interface inter = shaders.useInterface();
        inter.setViewprot(shaders.getUniformLocation("projection"), 0, window->width(), 0, window->height());
    }

    void translation(const TranslationInfo &info) override
    {
        viewpro.set(info.size, info.pos);

        arrow[0].setPos(info.size.width()/2 - arrow[0].width()/2, info.size.height()/2 - arrow[0].height()/2 + 60);
        arrow[1].setPos(info.size.width()/2 - arrow[0].width()/2 - 60, info.size.height()/2 - arrow[0].height()/2 );
        arrow[2].setPos(info.size.width()/2 - arrow[0].width()/2, info.size.height()/2 - arrow[0].height()/2 - 60);
        arrow[3].setPos(info.size.width()/2 - arrow[0].width()/2 + 60, info.size.height()/2 - arrow[0].height()/2);
        texts.setPosX(info.size.width() - texts.width() - 10);
        icon.setPosX(texts.x() - icon.width() - 5);

        { // 一个线程同一时间内只能有一个Interface对象
        RShaderProgram::Interface inter = RPlane::planeShader().useInterface();
        inter.setViewprot(pro, 0, info.size.width(), 0, info.size.height());
        }

        const RShaderProgram &shaders = RTextsbxo::textboxShader();
        RShaderProgram::Interface inter = shaders.useInterface();
        inter.setViewprot(shaders.getUniformLocation("projection"), 0, info.size.width(), 0, info.size.height());
    }

    void inputEvent(RInputEvent &e) override
    {
        // inputEvent只能监测感兴趣的按键
        if(e.press(Keys::KEY_ESCAPE))
            getParent()->breakLoop();

        RPoint2 p(0);
        if(e.status(Keys::KEY_LEFT) == ButtonAction::PRESS)
            p.rx() -= 4;
        if(e.status(Keys::KEY_RIGHT) == ButtonAction::PRESS)
            p.rx() += 4;
        if(e.status(Keys::KEY_UP) == ButtonAction::PRESS)
            p.ry() += 4;
        if(e.status(Keys::KEY_DOWN) == ButtonAction::PRESS)
            p.ry() -= 4;
        if(viewpro.contains(plane.rect() + p))
            plane.setPos(plane.pos() + p);
    }

private:
    RPlane plane;
    RPlane icon;
    RTextsbxo arrow[4];
    RTextsbxo texts;
    RRect viewpro;
    GLuint pro, view;
};

int main()
{
    RWindow::Format format;
    format.initWidth = 800;
    format.initHeight = 520;
    format.debug = true;
    format.background = 0x181010;
    RWindow window(format, nullptr, "Plane");

    TestCtl t(&window, "ChildNode");

    window.show();
    return window.exec();
}
