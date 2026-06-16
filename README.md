# Visualizador de Cenas 3D — Trabalho Prático GB

Trabalho Prático do Grau B da disciplina de Computação Gráfica (Unisinos).  
Visualizador de cenas 3D com OpenGL moderna, carregamento de múltiplos OBJs via arquivo JSON, iluminação de Phong e skybox.

**Componente:** Otto Schmitz

## Estado atual

Base do projeto implementada com:

- Leitura de cena via `src/config.json`
- Carregamento de OBJ com grupos (`g`/`o`), materiais `.mtl` e textura difusa
- Iluminação de Phong com múltiplas fontes de luz
- Câmera livre (WASD + mouse)
- Skybox opcional de fundo
- Cena inicial: mapa `village_map` (export Mineways/Minecraft)

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

- Adicionar demais objetos do diorama (mínimo 15 na cena final)
- Seleção e transformações (rotação, translação, escala)
- Animação por curva paramétrica (Catmull-Rom ou Bézier)
