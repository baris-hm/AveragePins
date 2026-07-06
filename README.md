# 📌 AveragePins

AveragePins is a lightweight desktop application built with Qt that helps artists, researchers, and developers calculate the **average position of selected points across multiple images**. It provides pixel‑level precision with an intuitive interface for layering, pin placement, and set management.

---

## ✨ Features
- **Import multiple images** as editable layers
- **Layer management**: rename, reorder, toggle visibility
- **Zoom & pan** to pixel-level detail
- **Pin placement**: add pins on specific pixels
- **Pin sets**: group pins into sets for organization
- **Average calculation**: instantly compute the mean position of pins in each set
- **Editable pins**: move, delete, or reassign pins between sets
- **Clean Qt interface** with professional styling

---

## 🖼️ Example Workflow
1. Select a canvas size.
2. Import multiple photos (e.g., 20 shots of the same location).
3. Arrange layers and toggle visibility.
4. Zoom in to pixel level and place pins on objects of interest.
5. Create pin sets for different objects.
6. Hit **Calculate Average** → get output pins representing average positions.

---

## 🚀 Getting Started (for now)

### Prerequisites
- [Qt Creator](https://www.qt.io/download) (Qt 6 recommended)
- C++17 or later
- CMake (if building outside Qt Creator)

### Build Instructions
```bash
git clone https://github.com/baris-hm/AveragePins.git
cd AveragePins
mkdir build && cd build
cmake ..
make
./AveragePins
