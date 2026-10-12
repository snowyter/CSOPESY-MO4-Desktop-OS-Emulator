// UIManager.cpp
#include "UIManager.h"

#include <utility>

UIManager& UIManager::instance() {
    // Created the first time this runs, then reused forever.
    static UIManager manager;
    return manager;
}

void UIManager::registerWindow(std::shared_ptr<AWindow> window) {
    if (window && !find(window->id())) windows_.push_back(std::move(window));
}

AWindow* UIManager::find(const std::string& id) const {
    for (const auto& w : windows_) {
        if (w->id() == id) return w.get();
    }
    return nullptr;
}

void UIManager::show(const std::string& id) {
    if (AWindow* w = find(id)) w->show();
}

void UIManager::hide(const std::string& id) {
    if (AWindow* w = find(id)) w->hide();
}

bool UIManager::isShown(const std::string& id) const {
    AWindow* w = find(id);
    return w && w->isShown();
}

bool UIManager::isMinimized(const std::string& id) const {
    AWindow* w = find(id);
    return w && w->isMinimized();
}

void UIManager::activate(const std::string& id) {
    AWindow* w = find(id);
    if (!w) return;
    if (isActive(id)) w->minimize();  // already in front -> minimise
    else              w->show();      // closed, minimised or behind -> open + to front
}

void UIManager::renderAllWindows() {
    for (const auto& w : windows_) {
        if (w->isShown() && !w->isMinimized()) w->draw();
    }
}

void UIManager::clear() {
    windows_.clear();
}
