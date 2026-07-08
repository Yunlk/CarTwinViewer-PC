#pragma once

#include <SDL_image.h>
#include "common.h"

// 封装 SDL_Texture，自动管理生命周期
struct Texture {
    SDL_Texture* tex = nullptr;
    int w = 0, h = 0;

    Texture() = default;
    ~Texture() { if (tex) SDL_DestroyTexture(tex); }

    bool load(SDL_Renderer* renderer, const char* path) {
        SDL_Surface* surf = IMG_Load(path);
        if (!surf) return false;
        w = surf->w; h = surf->h;
        tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        return tex != nullptr;
    }

    void draw(SDL_Renderer* renderer, int x, int y) const {
        SDL_Rect dst = { x, y, w, h };
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
    }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& o) noexcept : tex(o.tex), w(o.w), h(o.h) {
        o.tex = nullptr;
    }
};
