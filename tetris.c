#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
// сдеать очистку памяти от фигур, которые уничтожились полностью в результате горизонтальных линий
#define SIZE 3
#define LEFT 0
#define RIGHT 1
#define SPEED 232
#define ROTATE 666
#define X_MAX 60
#define Y_MAX 90
#define FSIZE SIZE*SIZE*4
#define LINE 100
#define JLINE 101
#define LLINE 102
#define CUBE 103
#define SLINE 104
#define ZLINE 105
#define TLINE 106
// все фигуры из 4 блоков!!!



typedef struct figure
{
    int Med; // индекс "центральной" координаты 
    int OnFLoar;   // флаг фигуры, лежит ли она на полу(чтобы поворот и движение на неё не действовало)
    char shape; // форма фигуры
    int** FigCoord; // FigCoord[n][0] = y, FigCoord[n][1] = x;   // точки фигуры
    int FigSize; // количество точек фигуры
}Figure;

typedef struct mapa
{
    int CountFigurs; // количество фигур
    Figure* Figurs; //  массив фигур
}Map;
// центрировать фигуру
void Center(Figure* f){
    for(int i = 0; i < FSIZE; i++){
        f->FigCoord[i][1] += X_MAX/2-SIZE;
    }
}
// передвинуть фигуру на nx и zy
void MoveTo(Figure* f, int x, int y){ 
    for(int i = 0; i < FSIZE; i++){
        f->FigCoord[i][0] += y;
        f->FigCoord[i][1] += x;
    }
}
//проверка на столконовение при вращении
int CollisionForRotate(Figure* f1, Figure* f2){ 
        for(int i = 0; i < FSIZE; i++){
            for(int j = 0; j < f2->FigSize; j++){
                if((f1->FigCoord[i][0] == f2->FigCoord[j][0])&&(f1->FigCoord[i][1] == f2->FigCoord[j][1])){ // bottom
                    return 1;
                }
            //    if((f1->FigCoord[i][0] == f2->FigCoord[j][0])&&(f1->FigCoord[i][1] == f2->FigCoord[j][1])){ // top
            //        return 1;
            //    }
            //    if((f1->FigCoord[i][0] == f2->FigCoord[j][0])&&(f1->FigCoord[i][1] == f2->FigCoord[j][1])){ // right
            //        return 1;
            //    }
            //    if((f1->FigCoord[i][0] == f2->FigCoord[j][0])&&(f1->FigCoord[i][1] == f2->FigCoord[j][1])){ // left
            //        return 1;
            //    }
            }
    }
    return 0;
}

// rotate to right around point (x0;y0)
//Y = (x — x0) * sin(alpha) + (y — y0) * cos(alpha) + y0;
//X = (x — x0) * cos(alpha) — (y — y0) * sin(alpha) + x0;
// вращение по часовой стрелке на 90 градусов 
void RotateNinety(Figure* f, Map m, char direction){ 
    int mnoj1, mnoj2;
    switch(direction){
        case('R'):
            mnoj1 = -1; mnoj2 = 1;
            break;
        case('L'):
            mnoj1 = 1; mnoj2 = -1;
            break;
        default:
            break;
    }
    int index = m.CountFigurs;
    int minx, maxx, miny, maxy; // 4 края фигуры
    minx = f->FigCoord[0][1]; miny = f->FigCoord[0][0]; 
    maxx = f->FigCoord[0][1]; maxy = f->FigCoord[0][0]; 
    int med = f->Med; // координата "центра" фигуры
    int temp;
    int movex= 0, movey= 0; // сдвиг, если фигура у края
    for(int i = 0; i < FSIZE; i++){
        if(f->shape == 'O') break;
        temp = mnoj1*(f->FigCoord[i][0] - f->FigCoord[med][0]) + f->FigCoord[med][1];
        f->FigCoord[i][0] = mnoj2*(f->FigCoord[i][1] - f->FigCoord[med][1]) + f->FigCoord[med][0];
        f->FigCoord[i][1] = temp;        
        if(minx > f->FigCoord[i][1]){
            minx = f->FigCoord[i][1];
        }
        if(miny > f->FigCoord[i][0]){
            miny = f->FigCoord[i][0];
        }
        if(maxx < f->FigCoord[i][1]){
            maxx = f->FigCoord[i][1];
        }
        if(maxy < f->FigCoord[i][0]){
            maxy = f->FigCoord[i][0];
        }
    }
    movex= 0;movey= 0;
    if(minx < 0){
        movex = abs(minx);
    }
    if(maxx >= X_MAX){
        movex = -(maxx - X_MAX + 1)  - (maxx - X_MAX + 1)%SIZE;
    }
    if(miny < 0){
        movey = abs(miny);
    }
    if(maxy > Y_MAX){
        movey = -(maxy - Y_MAX + 1) - (maxy - Y_MAX +1)%SIZE;
    }
    MoveTo(f, movex, movey);
    for(int i = 0; i < index-1; i++){
        if(CollisionForRotate(f, &m.Figurs[i])){ 
            for(int j = 0; j < FSIZE; j++){
                temp = mnoj2*(f->FigCoord[j][0] - f->FigCoord[med][0]) + f->FigCoord[med][1];
                f->FigCoord[j][0] = mnoj1*(f->FigCoord[j][1] - f->FigCoord[med][1]) + f->FigCoord[med][0];
                f->FigCoord[j][1] = temp;
            }
            break;
        }
    }
}

int CollisionDown(Figure* f1, Figure* f2){ // проверка на столкновение при движении вниз
    for(int i = 0; i < FSIZE; i++){
        for(int j = 0; j < f2->FigSize; j++){
            if((f1->FigCoord[i][0]+1 == f2->FigCoord[j][0])&&(f1->FigCoord[i][1] == f2->FigCoord[j][1])){ // bottom
                return 1;
            }
        }
    }
    return 0;
}
int CollisionLeft(Figure* f1, Figure* f2){ // проверка на столкновение при движении в лево
    for(int i = 0; i < FSIZE; i++){
        for(int j = 0; j < f2->FigSize; j++){
            if((f1->FigCoord[i][0] == f2->FigCoord[j][0])&&(f1->FigCoord[i][1]-1 == f2->FigCoord[j][1])){ // left
                return 1;
            }
        }
    }
    return 0;
}
int CollisionRight(Figure* f1, Figure* f2){ // проверка на столкновение при движении в право
    for(int i = 0; i < FSIZE; i++){
        for(int j = 0; j < f2->FigSize; j++){
            if((f1->FigCoord[i][0] == f2->FigCoord[j][0])&&(f1->FigCoord[i][1]+1 == f2->FigCoord[j][1])){ // right
                return 1;
            }
        }
    }
    return 0;
}

void GenFig(Figure* f, char shape){
    int x= 0,y= 0;
    f->FigCoord = (int**)malloc(sizeof(int*) * FSIZE); // выделение памяти под координаты
    f->Med = 13;
    f->shape = shape;
    for(int i = 0; i < FSIZE ; i++){
        f->FigCoord[i] = (int*)malloc(sizeof(int) * 2);
    }
    for (int i = 0; i < FSIZE; i++){ // заполнение координат(левый верх) - сделать по центру, и при генерации фигуры просто смещать след.блок
        switch(shape){
            case('J'):
                if(i == SIZE*SIZE*3){
                    x = -(SIZE);
                    y = SIZE;
                }
                break;
            case('L'):
                if(i == SIZE*SIZE*3){
                    x = SIZE;
                    y = SIZE;
                }
                break;
            case('O'):
                if(i == SIZE*SIZE*2){
                    x = SIZE;
                    y = 2*SIZE;
                }
                break;
            case('S'):
                if((i == 2*SIZE*SIZE) || (i == 3*SIZE*SIZE)){
                    y = 2*SIZE;
                    x = pow(-1, i/SIZE)*SIZE;
                }
                break;
            case('T'):
                if((i == 2*SIZE*SIZE) || (i == 3*SIZE*SIZE)){
                    y = i/SIZE;
                    x = pow(-1, y)*SIZE;
                }
                break;
            case('Z'):
                if((i == 2*SIZE*SIZE) || (i == 3*SIZE*SIZE)){
                    y = 2*SIZE;
                    x = pow(-1, i/SIZE+1)*SIZE;
                }
                break;
            default:
                break;
        }
        f->FigCoord[i][0] = i/SIZE - y; // + смщение по y
        f->FigCoord[i][1] = i%SIZE + x; // + смещение по x(вправо влево)
    }
    Center(f);
}
void RandomGen(Figure* f){
    int r = rand()%7;
    switch(r){
        case(0):
            GenFig(f, 'I');
            break;
        case(1):
            GenFig(f, 'J');
            break;
        case(2):
            GenFig(f, 'L');
            break;
        case(3):
            GenFig(f, 'O');
            break;
        case(4):
            GenFig(f, 'S');
            break;
        case(5):
            GenFig(f, 'T');
            break;
        case(6):
            GenFig(f, 'Z');
            break;
        default:
            break;
    }
}

int Down(Figure* f, Map m){ // сдвиг вниз на 1 блок
    int index = m.CountFigurs;
    for(int i = 0; i < index-1; i++){ // проверка на столкновение с другими фигурами
        if(CollisionDown(f, &m.Figurs[i])){
            return 1;
        }
    }
    for(int i = 0; i < FSIZE; i++){ // проверка на столкновение со стенкой
        if(f->FigCoord[i][0] == Y_MAX-3){
            return 1;
        }
    }
    for(int i = 0; i < FSIZE; i++){ // движение
        f->FigCoord[i][0]+=SIZE;
    }
    return 0;
}

int Right(Figure* f, Map m){ // сдвиг в право на 1 блок
    int index = m.CountFigurs;
    for(int i = 0; i < index-1; i++){ // проверка на столкновение с другими фигурами
        if(CollisionRight(f, &m.Figurs[i])){
            return 1;
        }
    }
    for(int i = 0; i < FSIZE; i++){  // проверка на столкновение со стенкой
        if(f->FigCoord[i][1] == X_MAX-1){
            return 1;
        }
    }
    for(int i = 0; i < FSIZE; i++){ // движение
        f->FigCoord[i][1]+=SIZE;
    }
    return 0;
}
int Left(Figure* f, Map m){ // сдвиг влево на 1 блок
    int index = m.CountFigurs;
    for(int i = 0; i < index-1; i++){ // проверка на столкновение с другими фигурами
        if(CollisionLeft(f, &m.Figurs[i])){
            return 1;
        }
    }
    for(int i = 0; i < FSIZE; i++){  // проверка на столкновение со стенкой
        if(f->FigCoord[i][1] < 1){
            return 1;
        }
    }
    for(int i = 0; i < FSIZE; i++){ // движение
        f->FigCoord[i][1]-=SIZE;
    }
    return 0;
}
int prflag;
int  score, hightscore;
FILE* f;
void PrintMap(char map[][X_MAX]){ // вывод карты
    move(0,0);
    clrtoeol();
    printw(" ");
    for(int i = 0; i < X_MAX; i++){
        printw("_");
    }
    printw("\n");
    for(int i = 0; i < Y_MAX; i++){
        move(i+1,0);
        clrtoeol();
        printw("|");
        for(int j = 0; j < X_MAX; j ++){
            switch(map[i][j]){
                case('J'):
                    attron(COLOR_PAIR(JLINE));
                    printw(" ");
                    attroff(COLOR_PAIR(JLINE));
                    break;
                case('I'):
                    attron(COLOR_PAIR(LINE));
                    printw(" ");
                    attroff(COLOR_PAIR(LINE));
                    break;
                case('S'):
                    attron(COLOR_PAIR(SLINE));
                    printw(" ");
                    attroff(COLOR_PAIR(SLINE));
                    break;
                case('Z'):
                    attron(COLOR_PAIR(ZLINE));
                    printw(" ");
                    attroff(COLOR_PAIR(ZLINE));
                    break;
                case('T'):
                    attron(COLOR_PAIR(TLINE));
                    printw(" ");
                    attroff(COLOR_PAIR(TLINE));
                    break;
                case('L'):
                    attron(COLOR_PAIR(LLINE));
                    printw(" ");
                    attroff(COLOR_PAIR(LLINE));
                    break;
                case('O'):
                    attron(COLOR_PAIR(CUBE));
                    printw(" ");
                    attroff(COLOR_PAIR(CUBE));
                    break;
                case('.'):
                    printw(".");
                    break;
                default:
                    printw(" ");
                    break;
            }
        }
        printw("|\n");
    }
    move(Y_MAX + 1, 0);
    clrtoeol();
    printw(" ");
    for(int i = 0; i < X_MAX; i++){
        printw("-");
    }
    move(Y_MAX + 2, 0);
    if(prflag) printw("SCORE is %d\n HIGHTSCORE is %d", score, hightscore);
    else printw("SCORE is %d", score);
    refresh();
}

int Delete(char m[][X_MAX], int* point){ // проверка карты на то, есть ли там линия по горизонтали для удаления
    int fl, height= 0; // флаг и высота промежутка для удаления
    for(int i = 0; i < Y_MAX; i++){
        fl =1;
        for(int j = 0; j < X_MAX; j++){ // проверка строки на заполненность
            if(m[i][j] == ' '){
                fl = 0;
                break;
            }
        }
        if(fl){ // если она заполнена
            height+=1;
            *point = i;
        }
    }
    return height; 
}
int DeletePiceFigur(Figure* f, int start, int stop){ // удаление кусочка фигуры
    int i = 0;
    int last = f->FigSize; // колчество точек
    while(i < last){
        if((f->FigCoord[i][0] > start) && (f->FigCoord[i][0] <= stop)){ // если координата находится в кусочке, то её отбрасываем в конец и уменьшаем колличество координат
            f->FigCoord[i][0] = f->FigCoord[last-1][0];
            f->FigCoord[i][1] = f->FigCoord[last-1][1];
            last--;
            continue;
        }
        else if(f->FigCoord[i][0] <= start){ // если точка выше кусочка, то её сдвигаем на высоту кусочка
            f->FigCoord[i][0] += stop - start;
        }
        i++;
    }
    f->FigSize = last; // изменение данных о фигуре
}

void RemoveFigur(Map* m, int index){ // удаление фигуры из списка  !!!(надобность под вопросом)!!!
    for(int i = 0; i < m->CountFigurs; i++){
        if(i == index){
            m->Figurs[i] = m->Figurs[m->CountFigurs-1];
            m->CountFigurs--;
            break;
        }
    }
}

Map  m;    // карта фигур
Figure Proection;
int  index_main;
char map[Y_MAX][X_MAX]; // карта с фигурами
int  KeyFlag, ExitFlag;
char Change;
int  speed, modul;

pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t figurs_mutex = PTHREAD_MUTEX_INITIALIZER;
void FillMap(char mapa[][X_MAX], Map m, Figure Prc){
    for(int i = 0; i < Y_MAX; i++){
        for(int j = 0; j < X_MAX; j ++){
             mapa[i][j] = ' ';
        }
    }
    for(int i = 0; i < Prc.FigSize; i++){
        mapa[Prc.FigCoord[i][0]][Prc.FigCoord[i][1]] = '.';
    }
    for(int i = 0; i < index_main; i++){ // заполнение карты фигурами
        for (int j = 0; j < m.Figurs[i].FigSize; j++){
            mapa[m.Figurs[i].FigCoord[j][0]][m.Figurs[i].FigCoord[j][1]] = m.Figurs[i].shape;
        }
    }
}
void CopyFigure(Figure* f1, Figure* f2){
    f1->shape = f2->shape;
    f1->Med = f2->Med;
    f1->OnFLoar = f2->OnFLoar;
    f1->FigSize = f2->FigSize;
    for(int i = 0; i < f1->FigSize ; i++){
        f1->FigCoord[i][0] = f2->FigCoord[i][0];
        f1->FigCoord[i][1] = f2->FigCoord[i][1];
    }
}
void* KeyHandlerThread(void* arg){ // поток обработки клавиш
    while(1){
        if(ExitFlag){
            pthread_mutex_unlock(&figurs_mutex);
            pthread_mutex_unlock(&map_mutex);
            return NULL;
        }
        Change = getch();
        pthread_mutex_lock(&map_mutex);
        pthread_mutex_lock(&figurs_mutex);
        if(Change == 'a'){
            KeyFlag = LEFT;
            Left(&m.Figurs[index_main-1], m);
            CopyFigure(&Proection, &m.Figurs[index_main-1]);
            while(!Down(&Proection, m)); // проекция опускается вниз
            speed = 0;
        }
        if(Change == 'd'){
            KeyFlag = RIGHT;
            Right(&m.Figurs[index_main-1], m);
            CopyFigure(&Proection, &m.Figurs[index_main-1]);
            while(!Down(&Proection, m)); // проекция опускается вниз
            speed = 0;
        } 
        if(Change == 'e'){ 
            KeyFlag = ROTATE;
            RotateNinety(&m.Figurs[index_main-1], m, 'R');
            CopyFigure(&Proection, &m.Figurs[index_main-1]);
            while(!Down(&Proection, m)); // проекция опускается вниз
            speed = 0;
        }
        if(Change == 'q'){
            KeyFlag = ROTATE;
            RotateNinety(&m.Figurs[index_main-1], m, 'L');
            CopyFigure(&Proection, &m.Figurs[index_main-1]);
            while(!Down(&Proection, m)); // проекция опускается вниз
            speed = 0;
        }
        if(Change == ' '){
            KeyFlag = SPEED;
            while(!Down(&m.Figurs[index_main-1], m));// fall down
            speed = 0;
        }
        if(Change == 'Q'){ // flag to quit
            ExitFlag = 1;
            if(score > hightscore){
                f = fopen("/home/aladin/.tetris_score","w");
                fprintf(f, "%d\n", score);
                fclose(f);
                f = NULL;
            }
            pthread_mutex_unlock(&figurs_mutex);// Освободите мьютексы перед завершением потока
            pthread_mutex_unlock(&map_mutex);
            return NULL;
        }
        FillMap(map, m, Proection);
        PrintMap(map);
        pthread_mutex_unlock(&figurs_mutex);
        pthread_mutex_unlock(&map_mutex);
   }
     return NULL;
}
void* MoveDownOnlyFig(void* arg){
    score = 0;
    int delheight = 0, startdel = 0, stopdel = 0; // высота удаляемого кусочка, начало и конец
    while(1){
        if(ExitFlag){
            getch();
            pthread_mutex_unlock(&figurs_mutex);
            pthread_mutex_unlock(&map_mutex);
            return NULL;
        }
        usleep(speed); 
        speed = 300000-modul;
        pthread_mutex_lock(&map_mutex);
        pthread_mutex_lock(&figurs_mutex);
        if(Down(&m.Figurs[index_main-1], m)){ // если фигура упала
            delheight = Delete(map, &stopdel); // записали высоту горизонтальной линии
            if(delheight){ // если есть горизонтальная линия
                score+=delheight + m.CountFigurs;
                startdel = stopdel - delheight; // вычисляем начало
                for(int i = 0; i < index_main; i++){ // удаляем кусочки во всех фигурах
                    DeletePiceFigur(&m.Figurs[i], startdel, stopdel);        
                }
            }
            if(m.Figurs[index_main-1].FigCoord[0][0] == 0){ // если места больше нет(конец игры)
                getch();
                clear();
                refresh();
                ExitFlag = 1;
                pthread_mutex_unlock(&map_mutex);
                pthread_mutex_unlock(&figurs_mutex);
                if(score > hightscore){
                    f = fopen("/home/aladin/.tetris_score","w");
                    fprintf(f, "%d ", score);
                    fclose(f);
                    f = NULL;
                }
                return NULL;
            }
            index_main++;
            m.CountFigurs++;
            m.Figurs = (Figure*)realloc(m.Figurs, sizeof(Figure)*index_main);
            m.Figurs[index_main-1].FigSize = FSIZE;
            RandomGen(&m.Figurs[index_main-1]);
            CopyFigure(&Proection, &m.Figurs[index_main-1]);
            while(!Down(&Proection, m)); // проекция опускается вниз
            if(m.CountFigurs % 5 == 0){ // с таким изменением скорости слишком сложно играть
                modul += 10000; 
            }
        }
        FillMap(map, m, Proection);
        PrintMap(map);
        pthread_mutex_unlock(&map_mutex);
        pthread_mutex_unlock(&figurs_mutex);
    }
    return NULL;
}
int main(){
    srand(time(NULL));
    initscr();
    start_color();
    init_pair(LINE, COLOR_WHITE, 45);
    init_pair(JLINE, COLOR_WHITE, 21);
    init_pair(LLINE, COLOR_WHITE, 136);
    init_pair(TLINE, COLOR_WHITE, 93);
    init_pair(SLINE, COLOR_WHITE, COLOR_GREEN);
    init_pair(ZLINE, COLOR_WHITE, COLOR_RED);
    init_pair(CUBE, COLOR_WHITE, 226);
    noecho();
    index_main=1;
    modul = 0;
    f = fopen("/home/aladin/.tetris_score","r");
    if(f == NULL){
        f = fopen("/home/aladin/.tetris_score","w");
        prflag = 0;
        hightscore = 0;
    }
    else{
        prflag = 1;
        fscanf(f, "%d", &hightscore);
    }
    m.Figurs = (Figure*)malloc(sizeof(Figure)*index_main); // инициализация 1 фигуры
    m.CountFigurs=1;
    m.Figurs[index_main-1].FigSize = FSIZE;
    RandomGen(&m.Figurs[index_main-1]); // генерация 1 фигуры
    Proection.FigCoord = (int**) malloc(sizeof(int*) * FSIZE); // выделение памяти под проекцию
    for(int i = 0; i < FSIZE; i++){
        Proection.FigCoord[i] = (int*) malloc(sizeof(int)*2);
    }
    CopyFigure(&Proection, &m.Figurs[index_main-1]);
    while(!Down(&Proection, m)); // проекция опускается вниз

    speed = 300000;
    pthread_t keyboarding_handler, move_down;
    if (pthread_create(&move_down, NULL, MoveDownOnlyFig , NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    if (pthread_create(&keyboarding_handler, NULL, KeyHandlerThread , NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    endwin();
    pthread_join(keyboarding_handler, NULL);
    pthread_join(move_down, NULL);
    system("clear");
    printf("End game. Score is %d, hightscore is %d\n", score, hightscore);
    if(score > hightscore){
        printf("You so cool, you up you hightscore!!!\n");
    }
    if(f != NULL) fclose(f);
    return 0;
}
