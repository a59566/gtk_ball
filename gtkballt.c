#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#define max_balls 100

int ballcount = 5;

int i;
int ball_x[max_balls] = {0};           //球圓心初始x座標
int ball_y[max_balls] = {0};           //球圓心初始y座標
int r[max_balls] = {0};                //球半徑
int x_vec[max_balls] = {0};            //球移動x向量
int y_vec[max_balls] = {0};            //球移動y向量

int board_x = 0;           //板子左上x座標
int board_y = 0;         //板子左上y座標
int board_width = 0;      //板子寬度
int board_height = 0;     //板子厚度

int windows_height = 300;   //視窗高度
int windows_weight = 400;   //視窗寬度

int speed = 100;
int board_move = 10;
gint timeout_id;
int game_over = TRUE;      //是否結束
int pause = FALSE;

GtkWidget *title;
GtkWidget *start;
GtkWidget *quit;
GtkWidget *table;
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
        //到左邊界
        if( ball_x[i] - r[i] <= 0 )
        {
             x_vec[i] = -x_vec[i]+(rand()%5-2);
             ball_x[i] += x_vec[i]/2;
             ball_y[i] += y_vec[i]/2;
        }
        //到右邊界
        if( ball_x[i] + r[i] >= windows_weight )
        {
            x_vec[i] = -x_vec[i]+(rand()%5-2);
            ball_x[i] += x_vec[i]/2;
            ball_y[i] += y_vec[i]/2;
        }
            
        //到上邊界
        if( ball_y[i] - r[i] <= 20)
        {
            y_vec[i] = -y_vec[i]+(rand()%5-2);
            ball_x[i] += x_vec[i]/2;
            ball_y[i] += y_vec[i]/2;
        }
            
        //到下邊界
        if( ball_y[i] + r[i] >= windows_height)
        {
            y_vec[i] = -y_vec[i]+(rand()%5-2);
            ball_x[i] += x_vec[i]/2;
            ball_y[i] += y_vec[i]/2;
        }
            

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
            }
            break;
        case 65364: //方向鍵 ↓
            g_source_remove(timeout_id);
            timeout_id = g_timeout_add(speed+=20, (GSourceFunc) time_handler, (gpointer) window);
            break;
        case 44:   //','
            if(ballcount > 5)
                ballcount--;
            break;
        case 46:   //'.'
            if(ballcount < max_balls)
            {
                ball_x[ballcount] =20;           //球圓心初始x座標
                ball_y[ballcount] =40;           //球圓心初始y座標
                r[ballcount] =10;               //球半徑
                x_vec[ballcount] = 10;            //球移動x向量
                y_vec[ballcount] = 10;            //球移動y向量
                ballcount++;
            }
            break;
        case 112:  //'p' 暫停控制
            if(pause)   //暫停取消
            {
                timeout_id = g_timeout_add(speed, (GSourceFunc) time_handler, (gpointer) window);
                pause = FALSE;
            }
            else    //暫停
            {
                g_source_remove(timeout_id);
                pause = TRUE;
            }
            break;
       case 110: //'n' new game
            new_game();
            g_source_remove(timeout_id);
            timeout_id = g_timeout_add(100, (GSourceFunc) time_handler, (gpointer) window);
            break;
       case 113: //'q' quit game
            gtk_main_quit ();
            break;
            
    }
}

void on_start(GtkWidget *widget,gpointer data)
{
    
    gtk_widget_hide (table);
    gtk_widget_hide (start);
    gtk_widget_hide (quit);
    gtk_widget_hide (title);
   
    new_game();
}

//開始，初始化遊戲
int new_game()
{
   
    for(i=0;i<ballcount;i++)
    {
        ball_x[i] =20;           //球圓心初始x座標
        ball_y[i] =40;           //球圓心初始y座標
        r[i] =10;               //球半徑

        x_vec[i] = 10+i;            //球移動x向量
        y_vec[i] = 10+(ballcount-i);            //球移動y向量
    }
    
    board_width = 20;      //板子寬度
    board_height = 20;     //板子厚度
    
    board_x = 0;                                     //板子左上x座標
    board_y = windows_height - board_height;         //板子左上y座標
    

    //將球數和速度回到預設
    speed = 100;
    ballcount = 5;
    
    
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

   /* 作出標題畫面 */
    table = gtk_table_new (5, 5, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);

    title = gtk_label_new("BALL GAME\n',.'to change ballcount\narrowkey to control&change speed\n'N'to newgame 'Q'to quit");
    gtk_table_attach_defaults (GTK_TABLE (table), title, 1, 4, 0, 1);
    gtk_widget_show (title);

    start = gtk_button_new_with_label ("START");
    g_signal_connect (G_OBJECT (start), "clicked",G_CALLBACK (on_start), (gpointer) "start_button");
    gtk_table_attach_defaults (GTK_TABLE (table), start, 1, 4, 2, 3);
    gtk_widget_show (start);

    quit = gtk_button_new_with_label ("Quit");
    g_signal_connect (G_OBJECT (quit), "clicked",G_CALLBACK (gtk_main_quit), NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), quit, 1, 4, 3, 4);
    gtk_widget_show (quit);
    /* 作出標題畫面 */
    
    //加入開始選項對應的處理函式
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_show_all(window);
    //加入time handler, 100ms
    timeout_id = g_timeout_add(speed, (GSourceFunc) time_handler, (gpointer) window);
    time_handler(window);

    gtk_main();
    return 0;

}
