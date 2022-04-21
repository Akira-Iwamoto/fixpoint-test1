#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define L 34
#define N 2
#define m 2
#define t 3
struct monitor_log {
    char date[20];
    char   ip[20];
    char ping[10];
    struct monitor_log *next;
};
struct server_log {
    char   ip[20];
    char address[40];
    struct server_log *next;
    struct monitor_log *ml;
};
void print_server_log(struct server_log *u) {
    struct server_log *v;
    struct monitor_log *l;
    v = u;
    while(u->next != NULL) {
        l = u->next->ml;
        while(l->next != NULL) {
            printf("%s ", l->next->date);
            printf("%s ", l->next->ip);
            printf("%s\n", l->next->ping);
            l = l->next;
        }
        printf("\n");
        u = u->next;
    }
    return;
}
void reset_char(int n, char s[n]) {
    int i;
    for(i=0;i<n;i++) s[i] = '\0';
    return;
}
void add_server(struct server_log *u, struct server_log *v) {
    struct server_log *s;
    struct monitor_log *l;
    s = u;
    while(s->next != NULL) s = s->next;
    v->next = s->next;
    s->next = v;
    l = (struct monitor_log*)malloc(sizeof(struct monitor_log));
    reset_char(20,l->date);
    reset_char(20,l->ip);
    reset_char(10,l->ping);
    l->next = NULL;
    v->ml = l;
    return;
}
void add_log(struct server_log *u, struct monitor_log *l) {
    struct server_log *v;
    struct monitor_log *r;
    v = u;
    while(strcmp(v->next->ip,l->ip) != 0) v = v->next;
    r = v->next->ml;
    while(r->next != NULL) r = r->next;
    l->next = r->next;
    r->next = l;
    return;
}
int same_server(struct server_log *u, struct server_log *v) {
    struct server_log *s;
    s = u;
    while(s->next != NULL) {
        if(strcmp(s->next->ip,v->ip) == 0) return 0; //既に登録したサーバー
        s = s->next;
    }
    return 1; //登録したことのないサーバー
}
int continue_timeout(struct monitor_log *x) {
    int n;
    struct monitor_log *y;
    y = x;
    n = 1;
    while(y->next != NULL) {
        if(y->next->ping[0] == '-') {
            n++;
        } else {
            break;
        }
        y = y->next;
    }
    if(n < N) return 0; //N回連続してタイムアウトはしていない
    return 1; //N回連続してタイムアウトした
}
void failure_period(struct server_log *u) {
    int c;
    struct server_log *v;
    struct monitor_log *l, *x, *y;
    v = u;
    printf("故障期間一覧\n");
    while(v->next != NULL) {
        l = v->next->ml;
        c = 0;
        while(l->next != NULL) {
            if(l->next->ping[0] == '-' && c == 0) {
                //故障期間が開始した
                x = l->next;
                if(continue_timeout(x) == 1) c = 1;
            } else if(l->next->ping[0] != '-' && c == 1) {
                //故障期間が終了した
                y = l->next;
                c = 2;
            }
            if(c == 2) {
                printf("サーバー: %s\n", x->ip);
                printf("故障期間: %s より ", x->date);
                printf("%s まで\n", y->date);
                c = 0;
            }
            l = l->next;
        }
        v = v->next;
    }
    printf("\n");
    return;
}
int response_time(struct monitor_log *l) {
    int i, total, to[m];
    double ave;
    struct monitor_log *r;
    r = l;
    i = 0;
    while(r->next != NULL) {
        i++;
        r = r->next;
    }
    if(i < m) return 0; //直近m回を調べられない
    r = l;
    for(i=0;i<m;i++) {
        if(r->next->ping[0] == '-') {
            to[i] = INT_MAX;
            //printf("- ");
        } else {
            to[i] = atoi(r->next->ping);
            //printf("%d ", to[i]);
        }
        r = r->next;
    }
    for(i=0,total=0;i<m;i++) {
        if(to[i] == INT_MAX) {
            printf("応答時間: ");
            for(i=0;i<m;i++) {
                if(to[i] == INT_MAX) printf("- ");
                else printf("%d ", to[i]);
            }
            printf("\n");
            return 1; //直近m回は過負荷状態
        }
        total = total + to[i];
    }
    ave = total / m;
    if(ave > t) {
        printf("応答時間: ");
        for(i=0;i<m;i++) {
            if(to[i] == INT_MAX) printf("- ");
            else printf("%d ", to[i]);
        }
        printf("\n");
        return 1; //直近m回は過負荷状態
    }
    return 0; //直近m回はtミリ秒を超えない
}
void overload_period(struct server_log *u) {
    int i;
    struct server_log *v;
    struct monitor_log *l, *r;
    v = u;
    printf("過負荷状態期間一覧\n");
    while(v->next != NULL) {
        l = v->next->ml;
        while(l->next != NULL) {
            if(response_time(l) == 1) {
                printf("サーバー: %s\n", l->next->ip);
                printf("過負荷  : %s より ", l->next->date);
                r = l;
                i = 0;
                while(i++ < m) r = r->next;
                printf("%s まで\n", r->date);
            }
            l = l->next;
        }
        v = v->next;
    }
    return;
}
int main(void){
    int i, j, k;
    char str[L][100];
    struct monitor_log *l;
    struct server_log *u, *v;
    for(i=0;i<L;i++) scanf("%s", str[i]);
    u = (struct server_log*)malloc(sizeof(struct server_log));
    reset_char(20,u->ip);
    u->next = NULL;
    u->ml   = NULL;
    for(i=0;i<L;i++) {
        v = (struct server_log*)malloc(sizeof(struct server_log));
        j = 0;
        while(str[i][j] != ',') j++;
        j++;
        k = 0;
        while(str[i][j] != ',') v->ip[k++] = str[i][j++];
        if(same_server(u,v) == 1) add_server(u,v); //登録したことのないサーバーなら登録する
    }
    for(i=0;i<L;i++) {
        l = (struct monitor_log*)malloc(sizeof(struct monitor_log));
        j = 0;
        while(str[i][j] != ',') {
            l->date[j] = str[i][j];
            j++;
        }
        j++;
        k = 0;
        while(str[i][j] != ',') l->ip[k++] = str[i][j++];
        j++;
        k = 0;
        while(str[i][j] != '\0') l->ping[k++] = str[i][j++];
        add_log(u,l);
    }
    print_server_log(u);
    failure_period(u);
    overload_period(u);
    return 0;
}
