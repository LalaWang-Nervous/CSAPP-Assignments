/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#define min(a,b) ((a) < (b) ? a : b)

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */


char transpose_submit_desc_32[] = "Transpose submission 32";
void transpose_submit_32(int M, int N, int A[N][M], int B[M][N])
{
    for(int i=0;i<(M/8);i++){
        for(int j=0;j<(N/8);j++){
            for(int k=0;k<8;k++){
//                for(int m=0;m<8;m++){
//                    int tmp = A[i*8+k][j*8+m];
//                    B[j*8+m][i*8+k] = tmp;
//                }
                int tmp0 = A[i*8+k][j*8+0];
                int tmp1 = A[i*8+k][j*8+1];
                int tmp2 = A[i*8+k][j*8+2];
                int tmp3 = A[i*8+k][j*8+3];
                int tmp4 = A[i*8+k][j*8+4];
                int tmp5 = A[i*8+k][j*8+5];
                int tmp6 = A[i*8+k][j*8+6];
                int tmp7 = A[i*8+k][j*8+7];
                B[j*8+0][i*8+k] = tmp0;
                B[j*8+1][i*8+k] = tmp1;
                B[j*8+2][i*8+k] = tmp2;
                B[j*8+3][i*8+k] = tmp3;
                B[j*8+4][i*8+k] = tmp4;
                B[j*8+5][i*8+k] = tmp5;
                B[j*8+6][i*8+k] = tmp6;
                B[j*8+7][i*8+k] = tmp7;
            }
        }
    }
}


/*
 * 64, 61' solution:
 * https://yangtau.me/computer-system/csapp-cache.html#64-x-64
 * */
char transpose_submit_desc_64[] = "Transpose submission 64";
void transpose_submit_64(int M, int N, int A[N][M], int B[M][N])
{
    int a0,a1,a2,a3,a4,a5,a6,a7;
    int tmp;
    int i,j,k;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (k = 0; k < 8 / 2; k++) {
                // A top left
                a0 = A[k + i][j];
                a1 = A[k + i][j + 1];
                a2 = A[k + i][j + 2];
                a3 = A[k + i][j + 3];

                // copy
                // A top right
                a4 = A[k + i][j + 4];
                a5 = A[k + i][j + 5];
                a6 = A[k + i][j + 6];
                a7 = A[k + i][j + 7];

                // B top left
                B[j][k + i] = a0;
                B[j + 1][k + i] = a1;
                B[j + 2][k + i] = a2;
                B[j + 3][k + i] = a3;

                // copy
                // B top right
                B[j + 0][k + 4 + i] = a4;
                B[j + 1][k + 4 + i] = a5;
                B[j + 2][k + 4 + i] = a6;
                B[j + 3][k + 4 + i] = a7;
            }
            for (k = 0; k < 8 / 2; k++) {
                // step 1 2
                a0 = A[i + 4][j + k]; a4 = A[i + 4][j + k + 4];
                a1 = A[i + 5][j + k]; a5 = A[i + 5][j + k + 4];
                a2 = A[i + 6][j + k]; a6 = A[i + 6][j + k + 4];
                a3 = A[i + 7][j + k]; a7 = A[i + 7][j + k + 4];
                // step 3
                tmp = B[j + k][i + 4]; B[j + k][i + 4] = a0; a0 = tmp;
                tmp = B[j + k][i + 5]; B[j + k][i + 5] = a1; a1 = tmp;
                tmp = B[j + k][i + 6]; B[j + k][i + 6] = a2; a2 = tmp;
                tmp = B[j + k][i + 7]; B[j + k][i + 7] = a3; a3 = tmp;
                // step 4
                B[j + k + 4][i + 0] = a0; B[j + k + 4][i + 4 + 0] = a4;
                B[j + k + 4][i + 1] = a1; B[j + k + 4][i + 4 + 1] = a5;
                B[j + k + 4][i + 2] = a2; B[j + k + 4][i + 4 + 2] = a6;
                B[j + k + 4][i + 3] = a3; B[j + k + 4][i + 4 + 3] = a7;
            }
        }
    }

}

char transpose_submit_desc_61[] = "Transpose submission 61";
void transpose_submit_61(int M, int N, int A[N][M], int B[M][N])
{
    int a0,a1,a2,a3,a4,a5,a6,a7;
    int i,j,s,k;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 23) {
            if ((i + 8 <= N) && (j + 23 <= M)) {
                for (s = j; s < j + 23; s++) {
                    a0 = A[i][s];
                    a1 = A[i + 1][s];
                    a2 = A[i + 2][s];
                    a3 = A[i + 3][s];
                    a4 = A[i + 4][s];
                    a5 = A[i + 5][s];
                    a6 = A[i + 6][s];
                    a7 = A[i + 7][s];
                    B[s][i + 0] = a0;
                    B[s][i + 1] = a1;
                    B[s][i + 2] = a2;
                    B[s][i + 3] = a3;
                    B[s][i + 4] = a4;
                    B[s][i + 5] = a5;
                    B[s][i + 6] = a6;
                    B[s][i + 7] = a7;
                }
            } else {
                for (k = i; k < min(i + 8, N); k++) {
                    for (s = j; s < min(j + 23, M); s++) {
                        B[s][k] = A[k][s];
                    }
                }
            }
        }
    }


}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M==32){
        transpose_submit_32(M,N,A,B);
        return;
    }
    else if(M==64){
        transpose_submit_64(M,N,A,B);
        return;
    }
    transpose_submit_61(M,N,A,B);
}
 /*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);


    /* Register any additional transpose functions */
//    registerTransFunction(trans, trans_desc);


}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

