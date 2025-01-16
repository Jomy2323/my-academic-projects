#
# IAC 2023/2024 k-means
# 
# Grupo:36
# Campus: Alameda
#
# Autores:
# 106883, Joao Silva
# 110580, Francisco Carvalho
# 107332, Guilherme Mendes
#
# Tecnico/ULisboa


# ALGUMA INFORMACAO ADICIONAL PARA CADA GRUPO:
# - A "LED matrix" deve ter um tamanho de 32 x 32
# - O input e' definido na seccao .data. 
# - Abaixo propomos alguns inputs possiveis. Para usar um dos inputs propostos, basta descomentar 
#   esse e comentar os restantes.
# - Encorajamos cada grupo a inventar e experimentar outros inputs.
# - Os vetores points e centroids estao na forma x0, y0, x1, y1, ...


# Variaveis em memoria
.data

# Valores de centroids, k e L a usar na 2a parte do prejeto:
centroids:   .word  0,0 24,31, 15,15
k:           .word 3
L:           .word 10

# Valores de centroids e k a usar na 1a parte do projeto:
#centroids:   .word 0,0
#k:           .word 1



# Abaixo devem ser declarados o vetor clusters (2a parte) e outras estruturas de dados
# que o grupo considere necessarias para a solucao:

### vetor clusters guarda os cluster_indexes de cada ponto em 1024 pontos possiveis
clusters: .zero 4096

colors:      .word 0xff0000, 0x00ff00, 0x0000ff  # Cores dos pontos do cluster 0, 1, 2, etc.

iteration_flag:   .byte 1

.equ         black      0
.equ         white      0xffffff 

#Input A - linha inclinada
#n_points:    .word 9
#points:      .word 0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 30,30 14,14

#Input B - Cruz
#n_points:    .word 5
#points:     .word 4,2, 5,1, 5,2, 5,3 6,2

#Input C
#n_points:    .word 23
#points: .word 0,0, 0,1, 0,2, 1,0, 1,1, 1,2, 1,3, 2,0, 2,1, 5,3, 6,2, 6,3, 6,4, 7,2, 7,3, 6,8, 6,9, 7,8, 8,7, 8,8, 8,9, 9,7, 9,8

#Input D
n_points:    .word 30
points:      .word 16, 1, 17, 2, 18, 6, 20, 3,  17, 4, 21, 7, 16, 4, 21, 6, 19, 6, 4, 24, 6, 24, 8, 23, 6, 26, 6, 26, 6, 23, 8, 25, 7, 26, 7, 20, 4, 21, 4, 10, 2, 10, 3, 11, 2, 12, 4, 13, 4, 9, 4, 9, 3, 8, 0, 10, 4, 10,21,1
     

# Codigo
 
.text
    # Chama funcao principal da 1a parte do projeto
    #jal mainSingleCluster
    
    # Descomentar na 2a parte do projeto:
    jal mainKMeans
    
    #Termina o programa (chamando chamada sistema)
    li a7, 10
    ecall

### printPoint
# Pinta o ponto (x,y) na LED matrix com a cor passada por argumento
# Nota: a implementacao desta funcao ja' e' fornecida pelos docentes
# E' uma funcao auxiliar que deve ser chamada pelas funcoes seguintes que pintam a LED matrix.
# Argumentos:
# a0: x
# a1: y
# a2: cor

printPoint:
    li a3, LED_MATRIX_0_HEIGHT #y
    sub a1, a3, a1
    addi a1, a1, -1
    li a3, LED_MATRIX_0_WIDTH #x
    mul a3, a3, a1
    add a3, a3, a0
    slli a3, a3, 2
    li a0, LED_MATRIX_0_BASE #address
    add a3, a3, a0   # addr
    sw a2, 0(a3)
    jr ra
    

### cleanScreen DONE
# Limpa todos os pontos do ecra
# Argumentos: nenhum
# Retorno: nenhum

cleanScreen:          
    addi sp, sp, -16  #Salvaguardar registos na stack
    sw s0, 0(sp)      #endereco atual da led matrix
    sw s1, 4(sp)      #dimensão da matriz
    sw s2, 8(sp)      #cor
    sw ra, 12(sp)     #registo ra  
    
    li s0, LED_MATRIX_0_BASE     # x = 32
    li s1, LED_MATRIX_0_HEIGHT   # y = 32
    li s2, white                 #cor para limpar o ecra
    mul s1, s1, s1               # x*y = 1024(n pontos a pintar)
    
###  for (n >= 0; n-=4)
loop_cleanScreen:    
    bge x0, s1, endLoop_cleanScreen  # if (0 >= n)
    sw s2, 0(s0)                     #pinta de branco 4 pontos
    sw s2, 4(s0)                     #de cada vez para melhorar       
    sw s2, 8(s0)                     #eficiencia
    sw s2, 12(s0)
    addi s1, s1, -4                  # n = n - 4 (menos quatro pontos)
    addi s0, s0, 16                  #avanca pointer da matriz 16 bytes  
    j loop_cleanScreen               #repete ciclo ate n = 0 
    
    endLoop_cleanScreen:    
        lw s0, 0(sp)    #Recuperar valores dos registos
        lw s1, 4(sp)
        lw s2, 8(sp)
        lw ra, 12(sp)
        addi sp, sp, 16
        jr ra    #return

###OPTIMIZACAO###
# Com o objetivo de optimizar o programa achamos que nao fazia sentido
# percorrer o vetor de pontos uma vez para os desenhar e obter o cluster
# index e outra vez para calcular os centroides. A nossa solucao consiste
# em usar um malloc na stack na funcao que se segue para ir somando os 
# valores de x, y e o numero de pontos de cada centroide. O segundo loop
# na funcao assim apenas tem de fazer k iteracoes para fazer o calculo das
# novas coordenadas e guardalas em memoria.
   
### printClusters_calculateNewCentroids
# Pinta os agrupamentos na LED matrix com a cor correspondente,
# e calcula as coordenadas dos novos centroides, metendo as
# coordenadas em memoria.
# Argumentos: nenhum
# Retorno: nenhum

printClusters_calculateNewCentroids:

    addi sp, sp, -32  #Salvaguardar registos na stack
    sw s0, 0(sp)      #x
    sw s1, 4(sp)      #y
    sw s2, 8(sp)      #n_elementos
    sw s3, 12(sp)     #endereco do input  
    sw s4, 16(sp)     #endereco cores
    sw s5, 20(sp)     #endereco clusters
    sw s6, 24(sp)     #para guardar tamanho do malloc
    sw ra, 28(sp)     #endereco de ra
    
    lw t0, k        #numero de centroides para alocar na stack
    li t1, 12       #cada centroide precisa de 3 words para guardar informacao
    mul t0, t0, t1  #tamanho a alocar na stack, cada centroide tera
    sub sp sp t0    #memoria dinamica alocada
    mv s6 t0        #guardar em s6 os bytes alocados para dar free no fim
    mv t6 sp        #t6 que nunca e usado vai ter sempre o valor de sp
                    #que aponta para base de malloc na stack
inicializar_0:
    ble t0 x0 done_0   #e preciso iniciar o malloc a 0
    sw x0 0(sp)        #fazemos 3 sw de cada vez porque 
    sw x0 4(sp)        #cada centroide precisa de 3 words
    sw x0 8(sp)
    addi t0 t0 -12
    addi sp sp 12
    j inicializar_0  
done_0:
    mv sp t6    
    lw s2, n_points   #numero de pontos a pintar
    la s3, points     #endereco das coordenadas dos pontos
    la s4, colors     #endereco das cores do centroide
    la s5, clusters   #endereco dos cluster_indexes de cada ponto  
    
    
### for (i = n_pontos; i > 0; i--)
loop_printClusters:
    bge x0, s2, endLoop_printClusters #em loop ate pintar todos
    lw s0, 0(s3)     #guardar x de ponto
    lw s1, 4(s3)     #guardar y de ponto
    addi s3, s3, 8   #preparar para apanhar proximo ponto
    addi s2, s2, -1  #menos 1 ponto para pintar (i--)
    mv a0, s0        #coordenada x em argumento a0     
    mv a1, s1        #coordenada y em argumento a1
    addi sp, sp, -8  #vamos proteger os registos a0 e a1
    sw a0, 0(sp)     #de acordo com as call conventions
    sw a1, 4(sp)     #do risc-v (callee)
    jal ra nearestCluster #apanha cluster index do ponto
    mv t0 a0           # index do cluster correspondente
    sw t0, 0(s5)       # guardar em memoria o index
    lw a0, 0(sp)       
    lw a1, 4(sp)
    addi sp, sp, 8
    
    slli t1, t0, 2     # index para aceder a cor do centroide certo
    add s4, s4, t1     # mudar o pointer para essa posicao
    lw a2, 0(s4)       # a2 fica com a cor pretendida
    sub s4, s4, t1     # volta a por o pointer no inicio do vetor para a proxima iteracao
    jal printPoint     #pintar o ponto de coordenadas (x,y) com a cor certa
    
    
    li t2 12
    mul t0 t0 t2       #t0 tem agora offset para guardar na memoria
    add t1 t6 t0       #t1 tem o pointer para o k(i) que quero calcular
    lw t3 0(t1)        #t3 apanha x
    lw t4 4(t1)        #t4 apanha y
    lw t5 8(t1)        #t5 apanha numero de pontos do cluster
    add t3 t3 s0       #adiciona x do ponto a sum_x
    add t4 t4 s1       #adiciona y do ponto a sum_y
    addi t5 t5 1       #n_pontos++
    sw t3 0(t1)        #guarda os valores atualizados na stack
    sw t4 4(t1)
    sw t5 8(t1)
    
    addi s5, s5, 4     # proximo centroide
    j loop_printClusters
endLoop_printClusters:
    lw t0 k            # t0 volta a ter n_centroides
    la t1 centroids    # vetor centroides
    li t5 0            # t5 a 0 significa que ainda nao se alteraram coordenadas
### for (i = k; i >= 0; i--)
loop_calculateNewCentroids:
    ble t0 x0  endLoop_calculateNewCentroids 
    lw t2 0(t6)                # sumX do centroide atual
    lw t3 4(t6)                # sumY do centroide atual
    lw t4 8(t6)                # n_pontos do centroide atual
    beq t4 x0 endIteration     # nenhum ponto pertence ao cluster, centroide fica igual
    div t2 t2 t4               # X = sumX / n_pontos
    div t3 t3 t4               # Y = sumY / n_pontos
    lw t4 0(t1)          # t4 tem a antiga coordenada do centroide
    beq t2 t4 proceedX
    li t5 1             # muda flag se alguma coordenadaX for alterada
proceedX:
    lw t4 4(t1)
    beq t3 t4 proceedY  # muda flag se alguma coordenadaY for alterada
    li t5 1
proceedY:
    sw t2 0(t1)         # guardar novo X do centroide
    sw t3 4(t1)         # guardar novo Y do centroide
endIteration:
    addi t0 t0 -1       # menos um centroide para calcular
    addi t1 t1 8        # proximo centroide a escrever coordenadas
    addi t6 t6 12       # nova posicao para aceder no malloc
    j loop_calculateNewCentroids
     
endLoop_calculateNewCentroids:    
    add sp sp s6        # free do malloc 
    la t0 iteration_flag
    sb t5 0(t0)
    
    lw s0, 0(sp)  
    lw s1, 4(sp)   
    lw s2, 8(sp)   
    lw s3, 12(sp) 
    lw s4, 16(sp) 
    lw s5, 20(sp)
    lw s6, 24(sp)
    lw ra, 28(sp)
    addi sp, sp, 32  # repor sp e registos saved
    jr ra


### printCentroids  DONE
# Pinta os centroides na LED matrix
# Nota: deve ser usada a cor preta (black) para todos os centroides
# Argumentos: nenhum
# Retorno: nenhum

printCentroids:

    addi sp sp -4      #salvaguarda ra
    sw ra 0(sp)
    
    li a2, black       #cor para todos os centroides
    la t0, centroids   #endereco centroides
    lw t1, k           #numero de centroides
    
loop_printCentroids:
    ble t1, x0, endLoop_printCentroids
    lw a0, 0(t0)        #apanha x
    lw a1, 4(t0)        #apanha y
    jal ra printPoint
    addi t1 t1 -1       #menos um centroide para pintar
    addi t0 t0 8
    j loop_printCentroids
endLoop_printCentroids:
    
    lw ra 0(sp)          #repoe ra
    addi sp sp 4
    jr ra
 
###OPTIMIZACAO###
# Em vez de limpar 1024 pontos da led matrix a cada iteracao
# apenas limpamos k pontos
  
### cleanCentroids  DONE
# Limpa os centroides na LED matrix
# Argumentos: nenhum
# Retorno: nenhum

cleanCentroids:

    addi sp sp -4      #salvaguarda ra
    sw ra 0(sp)
    
    li a2, white       #cor para todos os centroides
    la t0, centroids   #endereco centroides
    lw t1, k           #numero de centroides
    
loop_cleanCentroids:
    ble t1, x0, endLoop_cleanCentroids
    lw a0, 0(t0)        #apanha x
    lw a1, 4(t0)        #apanha y
    jal ra printPoint
    addi t1 t1 -1       #menos um centroide para limpar
    addi t0 t0 8
    j loop_cleanCentroids
endLoop_cleanCentroids:
    
    lw ra 0(sp)          #repoe ra
    addi sp sp 4
    jr ra
    
### manhattanDistance DONE
# Calcula a distancia de Manhattan entre (x0,y0) e (x1,y1)
# Argumentos:
# a0, a1: x0, y0
# a2, a3: x1, y1
# Retorno:
# a0: distance
# NOTA: nunca nesta funcao se altera o valor de a2 e a3
manhattanDistance:
    sub a0 a0 a2    # X = x0 - x1
    sub a1 a1 a3    # Y = y0 - y1
    blt a0 x0 absX  # X < 0 ? |X| : checkY
    j checkY
absX:
    neg a0 a0       # |X|
checkY:
    blt a1 x0 absY  # Y < 0 then |Y| else  proceed
    j proceed
absY:
    neg a1 a1       # |Y|
proceed:
    add a0 a0 a1    # |x0 - x1| + |y0 - y1| formula da distancia de manhattan
    jr ra
    

### nearestCluster DONE
# Determina o centroide mais perto de um dado ponto (x,y).
# Argumentos:
# a0, a1: (x, y) point
# Retorno:
# a0: cluster index

nearestCluster:
    addi sp sp -8
    sw ra 0(sp)
    sw s0 4(sp)        # s0 vai guardar o index do cluster a que pertence o ponto
    mv a2 a0           # passar coordenada x para a2 (de modo a facilitar chamada de...)
    mv a3 a1           # passar coordenada y para a3 (...funcao manhattanDistance)
    la t0 centroids    # vetor com coordenadas de centroides em t0
    lw t1 k            # numero de centroides
    li t2 0            # atual index de cluster
    li t3 62           # maior distancia possivel
loop_nearestCluster:
    ble t1 x0 endLoop_nearestCluster
    lw a0 0(t0)
    lw a1 4(t0)
    jal ra manhattanDistance
    bge a0 t3 skipValueUpdate
    mv t3 a0           # t3 tem agora o valor da menor distancia
    mv s0 t2           # s0 tem o index do cluster do ponto
skipValueUpdate:
    addi t1 t1 -1      # menos um centroide para calcular distancia
    addi t2 t2 1       # i++
    addi t0 t0 8       # coordenadas do proximo centroide
    j loop_nearestCluster
endLoop_nearestCluster:   
    mv a0 s0
    
    lw ra 0(sp)
    lw s0 4(sp)
    addi sp sp 8      # restaurar registos e atualizar stack pointer
    jr ra


### mainKMeans
# Executa o algoritmo *k-means*.
# Argumentos: nenhum
# Retorno: nenhum

mainKMeans:
    addi sp sp -8
    sw s0 0(sp)
    sw ra 4(sp)  
    
    lw s0 L                # numero de iteracoes a realizar
    
    jal ra initializeCentroids
    jal ra cleanScreen
loop_mainKmeans:
    lb t0 iteration_flag   # flag que se ativa corre mais uma iteracao
                           # comeca a 1
                           
    beq x0 t0 endLoop_mainKmeans # coordenadas dos centroides nao se alteraram END
    ble s0 x0 endLoop_mainKmeans # ja se fizeram as L iteracoes END  
    jal ra cleanCentroids        # limpa apenas os centroides
    jal ra printClusters_calculateNewCentroids # pintar clusters e calcular centroides
    jal ra printCentroids        # pintar novos centroides de preto (podem ser os mesmos que a iteracao anterior)
    addi s0 s0 -1                # menos uma iteracao a realizar
    j loop_mainKmeans
endLoop_mainKmeans:    
    lw s0 0(sp)
    lw ra 4(sp)
    addi sp sp 8
    jr ra
    

### initializeCentroids
# Gera coordenadas aleatorias para os k centroides.
# Argumentos: nenhum
# Retorno: nenhum    

initializeCentroids:
    li a7 30         #retorna epoch time (lower 32 bits)
    lw t0 k          # numero de centroides
    la t1 centroids  # endereco do vetor centroides
    li t2 0          # iterador
    li t3 32         # mod 32

loop_initializeCentroids:
    bge t2 t0 endLoop_initializeCentroids  # assim que i = k, salta
    ecall               #system call epoch time
    remu t4 a0 t3       #a0 mod 32
    sw t4 0(t1)         #guarda x
    ecall               #system call epoch time
    remu t4 a0 t3       #a0 mod 32
    sw t4 4(t1)         #guarda y
    addi t2 t2 1        #next centroid
    addi t1 t1 8        #address do novo centroide
    j loop_initializeCentroids
endLoop_initializeCentroids:
    jr ra