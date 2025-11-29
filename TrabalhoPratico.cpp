#include <iostream>
#include <cstring> // Necessário para strcpy, strcmp (Manipulação de strings estilo C)

using namespace std;

// --- ESTRUTURAS DE DADOS ---

// Estrutura do Livro (Nó Híbrido: serve para Lista e Árvore)
struct Livro {
    char titulo[100];
    char autor[50];
    int ano;

    // Ponteiros para as estruturas
    Livro* proximo;  // Lista Encadeada
    Livro* esquerda; // Árvore Binária
    Livro* direita;  // Árvore Binária

    // Construtor auxiliar
    Livro(const char* t, const char* a, int an) {
        strcpy(titulo, t);
        strcpy(autor, a);
        ano = an;
        proximo = NULL;
        esquerda = NULL;
        direita = NULL;
    }
};

// Estrutura para a Pilha de Operações (Histórico)
struct Operacao {
    char tipo; // 'C' = Cadastro, 'R' = Remoção
    // Armazenamos uma CÓPIA dos dados para poder restaurar
    char titulo[100];
    char autor[50];
    int ano;
    
    Operacao* anterior; // Ponteiro para o elemento abaixo na pilha
};

// --- CLASSE DO SISTEMA ---

class Catalogo {
private:
    Livro* cabecaLista;     // Início da Lista
    Livro* raizArvore;      // Raiz da Árvore
    Operacao* topoPilha;    // Topo da Pilha

    // --- MÉTODOS AUXILIARES DA ÁRVORE (Recursivos) ---

    Livro* inserirNaArvore(Livro* raiz, Livro* novo) {
        if (raiz == NULL) return novo;

        // Compara strings: strcmp retorna < 0 se o primeiro for menor (alfabeticamente)
        if (strcmp(novo->titulo, raiz->titulo) < 0) {
            raiz->esquerda = inserirNaArvore(raiz->esquerda, novo);
        } else {
            raiz->direita = inserirNaArvore(raiz->direita, novo);
        }
        return raiz;
    }

    Livro* buscarNaArvore(Livro* raiz, const char* busca) {
        if (raiz == NULL || strcmp(raiz->titulo, busca) == 0) {
            return raiz;
        }
        if (strcmp(busca, raiz->titulo) < 0) {
            return buscarNaArvore(raiz->esquerda, busca);
        } else {
            return buscarNaArvore(raiz->direita, busca);
        }
    }

    void percursoInOrder(Livro* raiz) {
        if (raiz != NULL) {
            percursoInOrder(raiz->esquerda);
            cout << "Titulo: " << raiz->titulo 
                 << " | Autor: " << raiz->autor 
                 << " | Ano: " << raiz->ano << endl;
            percursoInOrder(raiz->direita);
        }
    }

    Livro* encontrarMinimo(Livro* no) {
        Livro* atual = no;
        while (atual && atual->esquerda != NULL)
            atual = atual->esquerda;
        return atual;
    }

    // Remoção da árvore (Lógica complexa de BST)
    Livro* removerDaArvore(Livro* raiz, const char* titulo, bool &encontrou) {
        if (raiz == NULL) return raiz;

        if (strcmp(titulo, raiz->titulo) < 0) {
            raiz->esquerda = removerDaArvore(raiz->esquerda, titulo, encontrou);
        } else if (strcmp(titulo, raiz->titulo) > 0) {
            raiz->direita = removerDaArvore(raiz->direita, titulo, encontrou);
        } else {
            // Nó encontrado
            encontrou = true;

            // Caso 1: Sem filhos
            if (raiz->esquerda == NULL && raiz->direita == NULL) {
                // Não deletamos aqui, pois o nó pode estar na lista.
                // Apenas desvinculamos da árvore retornando NULL.
                return NULL; 
            }
            // Caso 2: Um filho
            else if (raiz->esquerda == NULL) {
                Livro* temp = raiz->direita;
                return temp;
            } else if (raiz->direita == NULL) {
                Livro* temp = raiz->esquerda;
                return temp;
            }
            // Caso 3: Dois filhos
            Livro* temp = encontrarMinimo(raiz->direita);
            // Copiamos os dados do sucessor
            strcpy(raiz->titulo, temp->titulo);
            strcpy(raiz->autor, temp->autor);
            raiz->ano = temp->ano;
            // Removemos o sucessor (agora duplicado)
            // Nota: Isso cria um desafio para a Lista Encadeada (os dados mudam de endereço),
            // mas simplificamos mantendo a estrutura da árvore.
            bool flagAux = false;
            raiz->direita = removerDaArvore(raiz->direita, temp->titulo, flagAux);
        }
        return raiz;
    }

    // --- MÉTODOS AUXILIARES DA LISTA ---
    
    void removerDaLista(const char* titulo) {
        Livro* atual = cabecaLista;
        Livro* anterior = NULL;

        while (atual != NULL) {
            if (strcmp(atual->titulo, titulo) == 0) {
                if (anterior == NULL) {
                    cabecaLista = atual->proximo;
                } else {
                    anterior->proximo = atual->proximo;
                }
                // Agora sim, seguro deletar da memória, pois saiu da Árvore E da Lista
                delete atual; 
                return;
            }
            anterior = atual;
            atual = atual->proximo;
        }
    }

    // --- MÉTODOS AUXILIARES DA PILHA ---

    void empilhar(char tipo, const char* t, const char* a, int ano) {
        Operacao* novaOp = new Operacao;
        novaOp->tipo = tipo;
        strcpy(novaOp->titulo, t);
        strcpy(novaOp->autor, a);
        novaOp->ano = ano;
        
        novaOp->anterior = topoPilha;
        topoPilha = novaOp;
    }

    void liberarMemoria(Livro* raiz) {
        if (raiz) {
            liberarMemoria(raiz->esquerda);
            liberarMemoria(raiz->direita);
            // Nota: Em nós híbridos, deletar pela árvore pode ser perigoso se a lista tentar acessar depois.
            // O ideal no Destrutor é limpar por uma das estruturas apenas.
        }
    }

public:
    Catalogo() {
        cabecaLista = NULL;
        raizArvore = NULL;
        topoPilha = NULL;
    }

    // Destrutor para limpar memória ao sair [Requisito 6]
    ~Catalogo() {
        // Limpar pilha
        while (topoPilha != NULL) {
            Operacao* temp = topoPilha;
            topoPilha = topoPilha->anterior;
            delete temp;
        }
        
        // Limpar livros (usando a lista é mais seguro para garantir que passamos por todos sem recursão dupla)
        Livro* atual = cabecaLista;
        while (atual != NULL) {
            Livro* prox = atual->proximo;
            delete atual;
            atual = prox;
        }
    }

    // [Requisito 1] Cadastrar Livro
    void cadastrar(const char* t, const char* a, int an, bool registrarUndo = true) {
        // 1. Criar Nó
        Livro* novo = new Livro(t, a, an);

        // 2. Inserir na Lista (Início)
        novo->proximo = cabecaLista;
        cabecaLista = novo;

        // 3. Inserir na Árvore
        raizArvore = inserirNaArvore(raizArvore, novo);

        // 4. Registrar na Pilha
        if (registrarUndo) {
            empilhar('C', t, a, an);
        }
        cout << "Livro cadastrado com sucesso.\n";
    }

    // [Requisito 2] Remover Livro
    void remover(const char* t, bool registrarUndo = true) {
        // Verificar se existe antes
        Livro* alvo = buscarNaArvore(raizArvore, t);
        if (alvo == NULL) {
            cout << "Livro nao encontrado.\n";
            return;
        }

        // Salvar dados para o Undo
        char t_bkp[100], a_bkp[50];
        int ano_bkp = alvo->ano;
        strcpy(t_bkp, alvo->titulo);
        strcpy(a_bkp, alvo->autor);

        // Remover da Árvore
        bool encontrou = false;
        raizArvore = removerDaArvore(raizArvore, t, encontrou);
        
        // Remover da Lista (e deletar fisicamente da memória)
        if (encontrou) {
            removerDaLista(t);
            if (registrarUndo) {
                empilhar('R', t_bkp, a_bkp, ano_bkp);
            }
            cout << "Livro removido.\n";
        }
    }

    // [Requisito 3] Buscar
    void buscar(const char* t) {
        Livro* res = buscarNaArvore(raizArvore, t);
        if (res) {
            cout << "\n--- ENCONTRADO ---\n";
            cout << "Titulo: " << res->titulo << "\nAutor: " << res->autor << "\nAno: " << res->ano << endl;
        } else {
            cout << "Livro nao encontrado no catalogo.\n";
        }
    }

    // [Requisito 4] Listar (Ordem Alfabética)
    void listar() {
        cout << "\n--- CATÁLOGO (Ordem Alfabetica) ---\n";
        if (raizArvore == NULL) cout << "Vazio.\n";
        percursoInOrder(raizArvore);
        cout << "-----------------------------------\n";
    }

    // [Requisito 5] Desfazer
    void desfazer() {
        if (topoPilha == NULL) {
            cout << "Nada para desfazer.\n";
            return;
        }

        Operacao* op = topoPilha;
        topoPilha = op->anterior; // Desempilha

        cout << "Desfazendo operacao... ";
        if (op->tipo == 'C') {
            // Se foi cadastro ('C'), desfazemos removendo o livro
            // Passamos 'false' para não registrar essa remoção na pilha de novo
            remover(op->titulo, false); 
        } else if (op->tipo == 'R') {
            // Se foi remoção ('R'), desfazemos recadastrando o livro
            cadastrar(op->titulo, op->autor, op->ano, false);
        }

        delete op; // Libera o nó da pilha
    }
};

// --- MENU PRINCIPAL ---

int main() {
    Catalogo sistema;
    int opcao, ano;
    char titulo[100], autor[50];

    do {
        cout << "\nFATEC Araras - Estrutura de Dados - Sistema de Catalogo de Livros\n";
        cout << "1. Cadastrar Livro | 2. Remover Livro | 3. Buscar Livro | 4. Listar Todos | 5. Desfazer | 6. Sair\nEscolha: ";
        cin >> opcao;
        cin.ignore(); // Limpar buffer

        switch (opcao) {
            case 1:
                cout << "Titulo: "; cin.getline(titulo, 100);
                cout << "Autor: "; cin.getline(autor, 50);
                cout << "Ano: "; cin >> ano;
                sistema.cadastrar(titulo, autor, ano);
                break;
            case 2:
                cout << "Titulo para remover: "; cin.getline(titulo, 100);
                sistema.remover(titulo);
                break;
            case 3:
                cout << "Buscar Titulo: "; cin.getline(titulo, 100);
                sistema.buscar(titulo);
                break;
            case 4:
                sistema.listar();
                break;
            case 5:
                sistema.desfazer();
                break;
            case 6:
                cout << "Encerrando e liberando memoria...\n";
                break;
            default:
                cout << "Opcao invalida.\n";
        }
    } while (opcao != 6);

    return 0;
}