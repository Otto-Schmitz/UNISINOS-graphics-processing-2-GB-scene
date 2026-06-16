# Visualizador de Cenas 3D — Trabalho Prático GB

Trabalho Prático do Grau B da disciplina de Computação Gráfica (Unisinos).  
Visualizador de cenas 3D com OpenGL moderna, carregamento de múltiplos OBJs via arquivo JSON, iluminação de Phong e skybox.

**Componente:** Otto Schmitz

## Estado atual

Base do projeto com cena **Vila Minecraft** (diorama):

| Elemento | Modelo | Qtd | Animação |
|----------|--------|-----|----------|
| Mapa | `village_map` | 1 | — |
| Ghast | `ghast/scene.obj` | 1 | Órbita Catmull-Rom sobre a vila |
| Happy Ghast | `happpy_ghast/scene.obj` | 4 | Pound (sobe/desce) |
| Allay | `allay/scene.obj` | 2 | Trajetórias Catmull-Rom |
| Allay | `allay/scene.obj` | 8 | Estáticos |
| Ghast | `ghast/scene.obj` | 2 | Estáticos |

**Total: 18 objetos na cena** (7 animados).

Implementado:

- Leitura de cena via `src/config.json`
- Carregamento de OBJ com grupos (`g`/`o`), materiais `.mtl` e textura difusa
- Iluminação de Phong com múltiplas fontes de luz
- Câmera livre (WASD + mouse)
- Skybox opcional de fundo
- Animação por curva **Catmull-Rom** (`src/catmull_rom.*`) com pontos de controle no JSON
- Cena inicial: mapa `village_map` + mobs Minecraft (ghast, happy ghast, allay)

## Dependências (Ubuntu/Debian)

```bash
sudo apt install build-essential cmake git libglfw3-dev libglm-dev libgl1-mesa-dev
```

## Compilação

Na pasta `proc_graf_2_GB_final`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Execução

```bash
./build/scene_viewer
```

Os caminhos de `assets/`, `shaders/` e `src/config.json` são resolvidos em tempo de compilação (`paths.h`).

## Controles

| Ação | Teclas |
|------|--------|
| Mover câmera | **W** **A** **S** **D** |
| Subir / descer | **Espaço** / **Ctrl esquerdo** |
| Olhar ao redor | **Mouse** |
| Selecionar objeto | **←** / **→** |
| Sair | **Esc** |

## Estrutura

- `src/config.json` — definição da cena (objetos, luzes, câmera)
- `src/obj_loader.*` — leitura de OBJ/MTL e upload na GPU
- `assets/village_map/` — mapa da vila (OBJ + textura atlas)
- `assets/skybox/` — faces do cubemap
- `shaders/phong.*` — iluminação Phong com textura
- `shaders/skybox.*` — fundo panorâmico

## Próximos passos

- Seleção e transformações (rotação, translação, escala) no objeto ativo
- Controles adicionais (wireframe, projeção ortográfica)
