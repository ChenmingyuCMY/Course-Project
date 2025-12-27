#include "GameWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("2D Game");
    QApplication::setApplicationDisplayName("2D Adventure Game");
    QApplication::setOrganizationName("GameDev");
    
    // 创建并显示游戏窗口
    GameWindow window;
    window.resize(800, 600);
    window.setWindowTitle("2D Adventure Game");
    window.show();
    
    return app.exec();
}