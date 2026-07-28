# Raptor V

**Raptor V** — лёгкий (почти) 3D-движок для Android. Написан на C++ с использованием Google Filament, Jolt Physics и EnTT ECS. Вся игровая логика и управление сценой вынесены в Kotlin/Java через JNI.

Цель проекта — предоставить гибкий API без захардкоженной логики, сочетая производительность C++ ядра и удобство Kotlin DSL.

---

## Стек технологий

* **Рендеринг:** [Google Filament](https://github.com/google/filament) (Vulkan / OpenGL ES 3.1)
* **Физика:** [Jolt Physics](https://github.com/jrouwe/JoltPhysics)
* **Архитектура (ECS):** [EnTT](https://github.com/skypjack/entt)
* **API и Логика:** Kotlin (Android JNI)

---

## Основные возможности

### Рендеринг и PBR
* **Модели шейдинга:** Base PBR, Clear Coat, Anisotropy, Sheen, Iridescence, Transmission & Dispersion, Subsurface Scattering (SSS), Emissive.
* **Постобработка:** GTAO (с поддержкой половинного разрешения для оптимизации на мобильных GPU), SSCT (контактные тени), SSR, TAA, Bloom, Depth of Field, Vignette, Fog, Dynamic Resolution (FSR).
* **Освещение:** Sun, Directional, Point, Spot. Каскадные тени, VSM, Soft Shadows и **Light Channels (0..7)** на уровне света и объектов.
* **Окружение:** IBL 3D Rotation, Cubemap / Panorama / HDR Skybox, физический диск солнца (`showSun`).

### Архитектура и API
* **Kotlin DSL:** Декларативная настройка графики (`postProcessing { ... }`) и материалов (`entity.material { ... }`).
* **Трансформации:** Иерархия Parent-Child, прямая работа с `position`, `rotation`, `scale` без ручного проброса массивов через JNI.
* **Оптимизация:** GPU Instancing (`InstanceBuffer`), общий буфер скелета для толп персонажей (`SkinningBuffer`), Bounding Box Culling.
* **Персонажи и Анимация:** GLTF-анимации с поддержкой cross-fade переходов, Morph Targets (блендшейпы мимики лица).
* **Процедурная геометрия:** `MeshBuilder` (генерация плоскостей, Point Clouds, Wireframe сеток из кода).
* **Частицы:** Компонент `ParticleEmitter` на базе GPU-инстансинга.
* **Математика:** Собственные реализации `Vec2`, `Vec3`, `Vec4`, `Quat` и `Mat4`.

---

## Пример использования (Kotlin)

```kotlin
val engine = Engine(context)
val scene = engine.createScene()
engine.setActiveScene(scene)

// Конфигурация графики
engine.postProcessing {
    colorGrading {
        toneMapping = ToneMappingMode.ACES
        exposure = 0.85f
        contrast = 1.2f
    }
    bloom {
        enabled = true
        strength = 0.08f
        lensFlare = true
    }
    ambientOcclusion {
        enabled = true
        type = AmbientOcclusionType.GTAO
        resolution = 0.5f // 50% разрешение чтоб телефон не бахнул
        enableSSCT = true
    }
}

// Создание объекта
val meshId = engine.loadMesh("DamagedHelmet.glb")
val helmet = scene.createEntity("Helmet")
helmet.attachMesh(meshId)
helmet.position = Vec3(0f, 1f, 0f)

// Настройка PBR-материала
helmet.material {
    clearCoat = 1.0f
    clearCoatRoughness = 0.05f
    roughness = 0.2f
    metallic = 0.8f
}
```

---

## Структура проекта

```text
├── raptorv/
│   ├── src/main/cpp/          # C++ ядро (Engine, Renderer, ResourceManager, Physics, JNI Bridge)
│   ├── src/main/java/         # Kotlin API (Engine, Scene, Entity, Material, Math, Pipeline)
└── sandbox/                   # Тестовый модуль (Песочница)
```

---

## Сборка

1. Требуется Android Studio с установленными NDK и CMake.
2. Клонируйте репозиторий: `git clone https://github.com/Danveyd/RaptorV.git`
3. Скомпилируйте и запустите модуль `:sandbox`.
* P.S. Возможно потребуется поместить свою модель в assets или переписать пример сцены в MainMenuActivity.kt, так как файл модели весил 153 мб и я не смог выложить его на GitHub
---
