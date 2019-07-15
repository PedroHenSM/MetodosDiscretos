#include "util.h"
#include "disk.h"
#include "inode.h"
#include "vfs.h"
#include "myfs.h"
#include <stdlib.h>
#include <stdbool.h>

//informações sobre o superbloco no setor 0
#define idSuperBloco sizeof(unsigned int)
#define numeroBlocosSuperBloco (3 * sizeof(unsigned int) + sizeof(char))
#define tamanhoSuperBloco 0
#define espacoLivreSuperBloco (sizeof(unsigned int) + sizeof(char))
#define primeiroBlocoSuperBloco (2 * sizeof(unsigned int) + sizeof(char))

#define bitInvalido 255
FileInfo* arquivos[MAX_FDS]; // array para armazenar os arquivos que estão em uso
FSInfo*f; // Ponteiro para o nosso pŕoprio sistema de arquivos

// Função para instalar o sistema de arquivos
int installMyFS (){
        FSInfo * fs = malloc (sizeof(FSInfo)); // aloco memória para a o sistema de aqruivo hipotético
        fs->fsid = 1; // id do sistema de arquivos hipotético
        fs->fsname = "lukinhadlc";     
        fs->isidleFn = isidleFn;  // fazendo a assimilação dos nomes das funções com o 
        fs->formatFn = formatFn; // respectivos ponteiros
        fs->openFn = openFn;
        fs->readFn = readFn;
        fs->writeFn = writeFn;
        fs->closeFn = closeFn;
        fs->opendirFn = opendirFn;
        fs->readdirFn = readdirFn;
        fs->linkFn = linkFn;
        fs->unlinkFn = unlinkFn;
        fs->closedirFn = closedirFn;
        vfsRegisterFS(fs); // fazendo registro do sistema de arquivo junto ao algoritmo
        f=fs;
}

//Funcao para verificacao se o sistema de arquivos está ocioso, ou seja,
//se nao ha quisquer descritores de arquivos em uso atualmente. Retorna
//um positivo se ocioso ou, caso contrario, 0.
int isidleFn (Disk *disco){
    int i;

    for(i=0; i < MAX_FDS; i++)
    {
        FileInfo* fileF = arquivos[i];
        if(diskGetId(d) == diskGetId(fileF->disk) && fileF != NULL) // Se existe arquivo em uso 
            return false;                                        // retorno falso
    }
 
    return true; // caso contrário retorno true

}


 //função para marcar o bloco como livre
bool marcarBlocoLivre(Disk *d, unsigned int block)
{
     unsigned char buffer[DISK_SECTORDATASIZE]; //configuro buffer com tamanho do setor do disco    

    if(buffer[idSuperBloco] != f.fsid) return false; //Se o disco não está formatado
                                                    // com o meu sistema de arquivos
                                                   // retorno false e não faço nada         
   
    if(diskReadSector(d, 0, buffer) == -1) return false; // Se o disco não está em com um
                                                        // formato válido no setor 0
                                                       // retorno false         
    unsigned int setoresBloco; // número de setores por bloco

    char2ul(&buffer[tamanhoSuperBloco], &setoresBloco);// faço conversão de endereços de char para long int
    setoresBloco /= DISK_SECTORDATASIZE;              // para que se possa fazer operações matemáticas, afim de 
                                                     // descobrir quantos   
    unsigned int numeroBlocos;
    char2ul(&buffer[numeroBlocosSuperBloco], &numeroBlocos); 

    unsigned int pBloco;
    char2ul(&buffer[primeiroBlocoSuperBloco ], &pBloco);

    unsigned int espacoLivrePorSetor;
    char2ul(&buffer[espacoLivreSuperBloco], &espacoLivrePorSetor);

    if((block - pBloco) / setoresBloco >= numeroBlocos) return false; // se tenho mais setores do que  blocos retorno false

    unsigned int espacoLivrepBloco = ((block - pBloco) / setoresBloco) / (DISK_SECTORDATASIZE * 8);
    if(diskReadSector(d, espacoLivrepBloco, buffer) == -1) return false; // se eu não posso ler este setor, retorno false
                                                                        
    unsigned int bitEspacoLivre = ((block - pBloco) / setoresBloco) % (DISK_SECTORDATASIZE * 8);
    buffer[bitEspacoLivre / 8] = setarBit0(buffer[bitEspacoLivre / 8], bitEspacoLivre % 8);

    if(diskWriteSector(d, espacoLivrepBloco, buffer) == -1) return false; // se não é um setor gravável retorno false

    return true;
}


unsigned int acharBlocoLivre(Disk *d)
{
    unsigned char buffer[DISK_SECTORDATASIZE];
    if(diskReadSector(d, 0, buffer) == -1) return -1;

    if(buffer[idSuperBloco] != f.fsid) return -1; // se o disco não está com o meu sistema de arquivos
                                                 // retorno false       
    unsigned int setoresBloco; // numero de setores por bloco
    char2ul(&buffer[tamanhoSuperBloco], &setoresBloco); // conversão de endereço
    setoresBloco /= DISK_SECTORDATASIZE; // calculo de numero de setores por bloco

    unsigned int numeroBlocos; // numero de bloco
    char2ul(&buffer[numeroBlocosSuperBloco], &numeroBlocos); // conversão de endereço

    unsigned int pBloco;
    char2ul(&buffer[primeiroBlocoSuperBloco ], &pBloco);

    unsigned int espacoLivrePSetor; 
    char2ul(&buffer[espacoLivreSuperBloco], &espacoLivrePSetor);

    unsigned int freeSpaceSize = pBloco - espacoLivrePSetor;

    unsigned int i;
    for(i = espacoLivrePSetor; i < espacoLivrePSetor + freeSpaceSize; i++)
    {
        if(diskReadSector(d, i, buffer) == -1) return -1;

        unsigned int j;
        for(j=0; j < DISK_SECTORDATASIZE; j++)
        {
            int freeBit = primeiroBitValido(buffer[j]);
            if(freeBit != -1)
            {
                unsigned int freeBlock = pBloco +
                             (i - espacoLivrePSetor) * DISK_SECTORDATASIZE * 8 * setoresBloco +
                             j * 8 * setoresBloco +
                             freeBit * setoresBloco;

                 if((freeBlock - pBloco) / setoresBloco >= numeroBlocos) return -1;

                buffer[j] = setarBit1(buffer[j], freeBit);
                if(diskWriteSector(d, i, buffer) == -1) return -1;

                return freeBlock;
            }
        }
    }

    return -1;
}


int primeiroBitValido(unsigned char byte)
{
    if(byte == bitInvalido) return -1;

    int i;
    unsigned char mascaraBits = 1;
    for(i=0; i < sizeof(unsigned char); i++)
    {
        if( (mascaraBits & byte) == 0 ) return i;
        mascaraBits <<= (unsigned char) 1;
    }
    return -1;
}



unsigned char setarBit1(unsigned char byte, unsigned int bit)
{
    unsigned char mascaraBits = (unsigned char) 1 << bit;
    return byte | mascaraBits;
}


unsigned char setarBit0(unsigned char byte, unsigned int bit)
{
    unsigned char mascaraBits = ((unsigned char) 1 << bit);
    mascaraBits = ~mascaraBits;
    return byte & mascaraBits;
}


//Funcao para formatacao de um disco com o novo sistema de arquivos
//com tamanho de blocos igual a blockSize. Retorna o numero total de
//blocos disponiveis no disco, se formatado com sucesso. Caso contrario,
//retorna -1.
int formatFn (Disk *d, unsigned int blockSize){

    unsigned char superblock[DISK_SECTORDATASIZE] = {0};

    ul2char(blockSize, &superblock[tamanhoSuperBloco]);
    superblock[idSuperBloco] = myfsInfo.fsid;

    unsigned int numInodes = (diskGetSize(d) / blockSize) / 8;

    unsigned int i;
    for(i=1; i <= numInodes; i++)
    {
        Inode* inode = inodeCreate(i, d);
        if(inode == NULL) return -1;
        free(inode);
    }

    unsigned int espacoLivrePSetor = inodeAreaBeginSector() + numInodes / inodeNumInodesPerSector();
    unsigned int freeSpaceSize   = (diskGetSize(d) / blockSize) / (sizeof(unsigned char) * 8 * DISK_SECTORDATASIZE);

    ul2char(espacoLivrePSetor, &superblock[espacoLivreSuperBloco]);

    unsigned int pBlocoSector = espacoLivrePSetor + freeSpaceSize;
    unsigned int numeroBlocos        = (diskGetNumSectors(d) - pBlocoSector) / (blockSize / DISK_SECTORDATASIZE);

    ul2char(pBlocoSector, &superblock[primeiroBlocoSuperBloco]);
    ul2char(numeroBlocos, &superblock[numeroBlocosSuperBloco]);

    if(diskWriteSector(d, 0, superblock) == -1 ) return -1;

    unsigned char freeSpace[DISK_SECTORDATASIZE] = {0};
    for(i=0; i < freeSpaceSize; i++)
    {
        if(diskWriteSector(d, espacoLivrePSetor + i, freeSpace) == -1) return -1;
    }

    if( numeroBlocos > 0)
       return -1;
   else
     return 0;
  }
    

//Funcao que escreve em *cyl o numero do cilindro correspondente a um endereco
//(addr) LBA de setor de um disco. Retorna 0 se o endereco for valido e -1
//caso contrario
int diskAddrToCylinder (Disk* d, unsigned long addr, unsigned long *cyl);    


//Funcao para abertura de um arquivo, a partir do caminho especificado
//em path, no disco montado especificado em d, no modo Read/Write,
//criando o arquivo se nao existir. Retorna um descritor de arquivo,
//em caso de sucesso. Retorna -1, caso contrario.
int openFn (Disk *d, const char *path){
        return -1;
}

//Funcao para a leitura de um arquivo, a partir de um descritor de
//arquivo existente. Os dados lidos sao copiados para buf e terao
//tamanho maximo de nbytes. Retorna o numero de bytes efetivamente
//lidos em caso de sucesso ou -1, caso contrario.
int readFn (int fd, char *buf, unsigned int nbytes){
     FileInfo* file = openFiles[fd];
    if(file == NULL) return -1;

    unsigned int tamArquivo = inodeGetFileSize(file->inode);
    unsigned int bytesLidos = 0;
    unsigned int inodeAtual = file->currentByte / file->diskBlockSize;
    unsigned int offset = file->currentByte % file->diskBlockSize;
    unsigned int blocoAtual = inodeGetBlockAddr(file->inode, currentInodeBlockNum);
    unsigned int bytesLidos = 0;
    unsigned char kBuffer[DISK_SECTORDATASIZE];

    while(bytesLidos < nbytes &&
          bytesLidos + file->currentByte < tamArquivo &&
          blocoAtual > 0)
    {
        unsigned int setoresBloco = file->diskBlockSize / DISK_SECTORDATASIZE;
        unsigned int primeiroSetor = offset / DISK_SECTORDATASIZE;
        unsigned int primeiroByteSetor = offset % DISK_SECTORDATASIZE;

        int i;
        for(i = primeiroSetor; i < setoresBloco; i++)
        {
            if(diskReadSector(file->disk,  blocoAtual+ i, KBuffer) == -1) return -1;

            int j;
            for(j = primeiroByteSetor;  j < DISK_SECTORDATASIZE &&
                                        bytesLidos < nbytes &&
                                        bytesLidos + file->currentByte < tamArquivo;  j++)
            {
                buf[bytesLidos] = kBuffer[j];
                bytesLidos++;
            }

            primeiroByteSetor = 0;
        }

        offset = 0;
        inodeAtual++;
        inodeAtual = inodeGetBlockAddr(file->inode, inodeAtual);
    }

    file->currentByte += bytesLidos;

    return bytesLidos;
}

//Funcao para a escrita de um arquivo, a partir de um descritor de
//arquivo existente. Os dados de buf serao copiados para o disco e
//terao tamanho maximo de nbytes. Retorna o numero de bytes
//efetivamente escritos em caso de sucesso ou -1, caso contrario
int writeFn (int fd, const char *buf, unsigned int nbytes){
        return -1;
}

//Funcao para fechar um arquivo, a partir de um descritor de arquivo
//existente. Retorna 0 caso bem sucedido, ou -1 caso contrario
int closeFn (int fd){
     FileInfo* f = arquivos[fd]; // recupero informações do sistema de arquivos

    if(f == NULL) return -1; // se ele está null retorno -1
  
     free(f->inode); //faço a desalocação do Inode

    free(f); // Faço a desalocação da RAM
    openFiles[fd] = NULL; // digo que esse descritor de arquivo não aponta para mais nada
    return 0;
}

//Funcao para abertura de um diretorio, a partir do caminho
//especificado em path, no disco indicado por d, no modo Read/Write,
//criando o diretorio se nao existir. Retorna um descritor de arquivo,
//em caso de sucesso. Retorna -1, caso contrario.
int opendirFn (Disk *d, const char *path){
        return -1;
}

//Funcao para a leitura de um diretorio, identificado por um descritor
//de arquivo existente. Os dados lidos correspondem a uma entrada de
//diretorio na posicao atual do cursor no diretorio. O nome da entrada
//e' copiado para filename, como uma string terminada em \0 (max 255+1).
//O numero do inode correspondente 'a entrada e' copiado para inumber.
//Retorna 1 se uma entrada foi lida, 0 se fim do diretorio ou -1 caso
//mal sucedido.
int readdirFn (int fd, char *filename, unsigned int *inumber){
 
    

}

//Funcao para adicionar uma entrada a um diretorio, identificado por um
//descritor de arquivo existente. A nova entrada tera' o nome indicado
//por filename e apontara' para o numero de i-node indicado por inumber.
//Retorna 0 caso bem sucedido, ou -1 caso contrario.
int linkFn (int fd, const char *filename, unsigned int inumber){
        return -1;
}

//Funcao para remover uma entrada existente em um diretorio,
//identificado por um descritor de arquivo existente. A entrada e'
//identificada pelo nome indicado em filename. Retorna 0 caso bem
//sucedido, ou -1 caso contrario.
int unlinkFn (int fd, const char *filename){
        return -1;
}

//Funcao para fechar um diretorio, identificado por um descritor de
//arquivo existente. Retorna 0 caso bem sucedido, ou -1 caso contrario.
int closedirFn (int fd){
        return -1;
}
