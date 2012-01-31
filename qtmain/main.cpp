#include <QDir>
#include <QTextStream>
#include <QStringList>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

QStringList stat()
{
    QDir dir("/proc/");
    dir.setFilter(QDir::Dirs | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QStringList result;

    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo dirInfo = list.at(i);
        QString path = QString("%1/stat").arg(dirInfo.filePath());

        QFile file(path);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QTextStream in(&file);
        QString data = in.readAll();

        QRegExp re("(\\d+)\\s+\\((.+)\\)\\s+([RSDZTW])\\s+(-?\\d+\\s+){10}(\\d+)");

        if (re.indexIn(data) != -1)
        {
            result << QString("PID: %1    CMD: %2    STATE: %3    UTIME: %4").arg(re.cap(1)).arg(re.cap(2)).arg(re.cap(3)).arg(re.cap(5)).toStdString().c_str();
        }
    }

    return result;
}

Display *     display;
int           screen_num;
static char * appname;

int main( int argc, char * argv[] )
{
    Window       win;
    int          x, y;
    unsigned int width, height;
    unsigned int border_width;
    char *       window_name = "Hello, X Window System!";
    char *       icon_name   = "HelloX";

    char *       display_name = NULL;
    unsigned int display_width, display_height;

    XSizeHints *  size_hints;
    XWMHints   *  wm_hints;
    XClassHint *  class_hints;
    XTextProperty windowName, iconName;
    XEvent        report;
    XFontStruct * font_info;
    XGCValues     values;
    GC            gc;

    appname = argv[0];

    if ( !( size_hints  = XAllocSizeHints() ) ||
         !( wm_hints    = XAllocWMHints()   ) ||
         !( class_hints = XAllocClassHint() )    ) {
        fprintf(stderr, "%s: couldn't allocate memory.\n", appname);
        exit(EXIT_FAILURE);
    }

    if ( (display = XOpenDisplay(display_name)) == NULL ) {
        fprintf(stderr, "%s: couldn't connect to X server %s\n",
                appname, display_name);
        exit(EXIT_FAILURE);
    }

    screen_num     = DefaultScreen(display);
    display_width  = DisplayWidth(display, screen_num);
    display_height = DisplayHeight(display, screen_num);

    x = y = 0;
    width  = display_width / 3;
    height = display_width / 3;

    win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                              x, y, width, height, border_width,
                              BlackPixel(display, screen_num),
                              WhitePixel(display, screen_num));

    if ( XStringListToTextProperty(&window_name, 1, &windowName) == 0 ) {
        fprintf(stderr, "%s: structure allocation for windowName failed.\n",
                appname);
        exit(EXIT_FAILURE);
    }

    if ( XStringListToTextProperty(&icon_name, 1, &iconName) == 0 ) {
        fprintf(stderr, "%s: structure allocation for iconName failed.\n",
                appname);
        exit(EXIT_FAILURE);
    }

    size_hints->flags       = PPosition | PSize | PMinSize;
    size_hints->min_width   = 200;
    size_hints->min_height  = 100;

    wm_hints->flags         = StateHint | InputHint;
    wm_hints->initial_state = NormalState;
    wm_hints->input         = True;

    class_hints->res_name   = appname;
    class_hints->res_class  = "hellox";

    XSetWMProperties(display, win, &windowName, &iconName, argv, argc,
                     size_hints, wm_hints, class_hints);

    XSelectInput(display, win, ExposureMask | KeyPressMask |
                 ButtonPressMask | StructureNotifyMask);

    if ( (font_info = XLoadQueryFont(display, "9x15")) == NULL ) {
        fprintf(stderr, "%s: cannot open 9x15 font.\n", appname);
        exit(EXIT_FAILURE);
    }

    gc = XCreateGC(display, win, 0, &values);

    XSetFont(display, gc, font_info->fid);
    XSetForeground(display, gc, BlackPixel(display, screen_num));

    XMapWindow(display, win);


    while ( 1 ) {
        static char * message = "Hello, X Window System!";
        //static int    length;
        static int    font_height;
        //static int    msg_x, msg_y;
        static int    msg_y = 0;

        XNextEvent(display, &report);

        switch ( report.type ) {

        case Expose:

            if ( report.xexpose.count != 0 )
                break;

        {
            /*length = XTextWidth(font_info, message, strlen(message));
            msg_x  = (width - length) / 2;*/

            msg_y = 0;
            QStringList list = stat();

            for (int i = 0; i < list.size(); i++)
            {
                font_height = font_info->ascent + font_info->descent;
                msg_y  += font_height + 2;

                XDrawString(display, win, gc, 10, msg_y,
                            list.at(i).toStdString().c_str(), list.at(i).length());
            }
        }
            break;


        case ConfigureNotify:
            width  = report.xconfigure.width;
            height = report.xconfigure.height;

            break;


        case DestroyNotify:
            XUnloadFont(display, font_info->fid);
            XFreeGC(display, gc);
            XCloseDisplay(display);
            exit(EXIT_SUCCESS);

        }
    }

    return 0;
}
