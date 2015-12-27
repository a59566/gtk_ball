#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#define max_balls 3

int ballcount = 1;

int i;
int ball_x[max_balls] = {0};           //球圓心初始x座標
int ball_y[max_balls] = {0};           //球圓心初始y座標
int r[max_balls] = {0};                //球半徑
int x_vec[max_balls] = {0};            //球移動x向量
int y_vec[max_balls] = {0};            //球移動y向量

int board_x = 0;           //板子左上x座標
int board_y = 280;         //板子左上y座標
int board_width = 20;      //板子寬度
int board_height = 20;     //板子厚度

int windows_height = 300;   //視窗高度
int windows_weight = 400;   //視窗寬度

int speed = 100;
int board_move = 10;
gint timeout_id;
int game_over = TRUE;      //是否結束
int pause = FALSE;
int speed_temp; 
int board_move_temp;


//繪製球
void draw_ball(cairo_t *cr)
{
    for(i=0;i<ballcount;i++)
    {
        cairo_arc(cr, ball_x[i], ball_y[i], r[i], 0, 2*M_PI);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_fill(cr);
    }
}
//繪製板子
void draw_slider(cairo_t *cr)
{
    cairo_rectangle(cr, board_x, board_y, board_width, board_height);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);
}
//螢幕刷新
static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr;
    cr = gdk_cairo_create (widget->window);
    draw_ball(cr);
    draw_slider(cr);
    cairo_destroy(cr);
    return FALSE;
}

//球的運動方式
void move_ball()
{
    for(i=0;i<ballcount;i++)
    {
        if( ball_x[i] - r[i] <= 0 )        x_vec[i] = -x_vec[i]+(rand()%5-2);//到左邊界
        if( ball_x[i] + r[i] >= windows_weight )        x_vec[i] = -x_vec[i]+(rand()%5-2);//到右邊界
        if( ball_y[i] - r[i] <= 20)        y_vec[i] = -y_vec[i]+(rand()%5-2);//到上邊界
        if( ball_y[i] + r[i] >= windows_height)        y_vec[i] = -y_vec[i]+(rand()%5-2);//到下邊界

        //板子上
        if( (ball_x[i] >= board_x) && (ball_x[i] <= board_x + board_width) )
        {
            if( ball_y[i] + r[i] >= board_y)
            {
                game_over = TRUE;
            }
        }
    }
    for(i=0;i<ballcount;i++)
    {
        ball_x[i] += x_vec[i];
        ball_y[i] += y_vec[i];
    }
}


//計時器
static gboolean time_handler(GtkWidget *widget)
{
    if (widget->window == NULL)
        return FALSE;

    if(!game_over)
    {
        move_ball();
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}
//按鍵控制
void on_key_press(GtkWindow *window, GdkEventKey *eventkey, gpointer data)
{
    switch(eventkey->keyval)
    {
        case 65361: //方向鍵 <-
            if( board_x - board_move >= 0 )
                board_x -= board_move;
            else
                board_x = 0;
            break;
        case 65363: //方向键 ->
            if( board_x + board_width + board_move <= windows_weight)
                board_x += board_move;
            else
                board_x = windows_weight - board_width;
            break;
        case 65362: //方向鍵 ↑    
            if(speed > 20)
            {
                g_source_remove(timeout_id);
                timeout_id = g_timeout_add(speed-=20, (GSourceFunc) time_handler, (gpointer) window);
                board_move += 10;  
            }   
            break;
        case 65364: //方向鍵 ↓
            g_source_remove(timeout_id);
            timeout_id = g_timeout_add(speed+=20, (GSourceFunc) time_handler, (gpointer) window);
            if(board_move >10)
                board_move -= 10;
            break;
        case 44:   //','
            if(ballcount > 1)
                ballcount--;
            break;
        case 46:   //'.'
            if(ballcount < max_balls)
                ballcount++;
            break;
        case 112:  //'p' 暫停控制
            if(pause)   //暫停取消
            {         
                g_source_remove(timeout_id);
                timeout_id = g_timeout_add(speed, (GSourceFunc) time_handler, (gpointer) window);
                pause = FALSE;
            }
            else    //暫停
            {              
                g_source_remove(timeout_id);
                pause = TRUE;
            }
            break;
    }
}


//開始，初始化遊戲
void on_start(GtkMenuItem *item,gpointer p)
{
    for(i=0;i<ballcount;i++)
    {
        ball_x[i] =20;           //球圓心初始x座標
        ball_y[i] =40;           //球圓心初始y座標
        r[i] =10;               //球半徑

        x_vec[i] = 10+i;            //球移動x向量
        y_vec[i] = 10+(ballcount-i);            //球移動y向量
    }

    board_x = 0;           //板子左上x座標
    board_y = 280;         //板子左上y座標
    board_width = 20;      //板子寬度
    board_height = 20;     //板子厚度
    
    speed = 100;
    
    game_over = FALSE;
}


int main(int argc,char **argv)
{
    srand(time(NULL));
    GtkWidget *window;
    gtk_init(&argc, &argv);

    //建立頂層視窗
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //增加按鍵處理
    gtk_widget_add_events(GTK_WIDGET(window), GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), NULL);

    //刷新事件
    g_signal_connect(G_OBJECT(window), "expose-event", G_CALLBACK(on_expose_event), NULL);
    //關閉程式事件
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //視窗居中
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    //設定視窗寬&高
    gtk_window_set_default_size(GTK_WINDOW(window), windows_weight, windows_height);
    GtkWidget *gamemenu;
    GtkWidget *menubar;
    GtkWidget *game;
    GtkWidget *start;
    GtkWidget *quit;
    GtkWidget *vbox;
    //建立vbox，把他加入容器內
    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(window),vbox);
    //建立主選單
    gamemenu = gtk_menu_new();
    //建立選單列
    menubar = gtk_menu_bar_new();
    //建立選單選項
    game = gtk_menu_item_new_with_label("Game");
    start = gtk_menu_item_new_with_label("Start");
    quit = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(game),gamemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(gamemenu),start);
    gtk_menu_shell_append(GTK_MENU_SHELL(gamemenu),quit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),game);

    gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE,FALSE,3);
    //加入開始選項對應的處理函式
    g_signal_connect(G_OBJECT(start), "activate", G_CALLBACK(on_start), NULL);
    
    g_signal_connect(G_OBJECT(quit), "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_show_all(window);
    //加入time handler, 100ms
    timeout_id = g_timeout_add(speed, (GSourceFunc) time_handler, (gpointer) window);
    time_handler(window);

    gtk_main();
    return 0;

}
