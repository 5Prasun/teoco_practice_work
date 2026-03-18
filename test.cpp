#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>

int main() {

    const char* inputFile = "abbotsdale5m_dhm.bin";
    int tileSize = 360;
    int bytesPerCell = sizeof(int16_t);

    // ---- Step 1: get file size ----
    std::ifstream file(inputFile, std::ios::binary | std::ios::ate);

    if (!file) {
        std::cout << "Cannot open file\n";
        return 1;
    }

    long fileSize = file.tellg();
    file.close();

    long cells = fileSize / bytesPerCell;

    std::cout << "File size: " << fileSize << " bytes\n";
    std::cout << "Total cells: " << cells << "\n";

    // ---- Step 2: detect raster size ----
    int width = 0;
    int height = 0;
    double bestDiff = 1e12;

    for (long w = 1; w <= std::sqrt(cells); w++) {

        if (cells % w == 0) {

            long h = cells / w;
            double diff = std::abs((double)w - (double)h);

            if (diff < bestDiff) {
                bestDiff = diff;
                width = w;
                height = h;
            }
        }
    }

    std::cout << "Detected raster size: " << width << " x " << height << "\n";

    // ---- Step 3: read raster ----
    std::ifstream in(inputFile, std::ios::binary);

    std::vector<int16_t> data(width * height);
    in.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(int16_t));
    in.close();

    // ---- Debug info ----
    int tilesX = (width + tileSize - 1) / tileSize;
    int tilesY = (height + tileSize - 1) / tileSize;

    std::cout << "Tiles in X direction: " << tilesX << std::endl;
    std::cout << "Tiles in Y direction: " << tilesY << std::endl;
    std::cout << "Total tiles expected: " << tilesX * tilesY << std::endl;

    // ---- Step 4: split into tiles ----
    for (int y = 0; y < height; y += tileSize) {
        for (int x = 0; x < width; x += tileSize) {

            int tileW = std::min(tileSize, width - x);
            int tileH = std::min(tileSize, height - y);

            std::vector<int16_t> tile(tileW * tileH);

            for (int j = 0; j < tileH; j++) {
                for (int i = 0; i < tileW; i++) {
                    tile[j * tileW + i] =
                        data[(y + j) * width + (x + i)];
                }
            }

            std::string outFile =
                "tile_" + std::to_string(x) + "_" +
                std::to_string(y) + ".bin";

            std::ofstream out(outFile, std::ios::binary);
            out.write(reinterpret_cast<char*>(tile.data()),
                      tile.size() * sizeof(int16_t));
            out.close();
        }
    }

    std::cout << "\nRaster tiling complete.\n\n";

    // ---------------------------------------------------
    // STEP 5: Process multiple towers
    // ---------------------------------------------------

int numTowers;

std::cout << "How many tower coordinates do you want to enter? ";
std::cin >> numTowers;

std::set<std::string> finalTiles;

for (int t = 0; t < numTowers; t++) {

    double lat, lon;

    std::cout << "Tower" << t+1 << ": ";
    std::cin >> lat >> lon;

    int pixelX = lon;
    int pixelY = lat;

    int tileX = (pixelX / tileSize) * tileSize;
    int tileY = (pixelY / tileSize) * tileSize;

    finalTiles.insert("tile_" + std::to_string(tileX) + "_" +
                      std::to_string(tileY) + ".bin");

    if (pixelX % tileSize == 0 && tileX > 0)
        finalTiles.insert("tile_" + std::to_string(tileX - tileSize) + "_" +
                          std::to_string(tileY) + ".bin");

    if (pixelY % tileSize == 0 && tileY > 0)
        finalTiles.insert("tile_" + std::to_string(tileX) + "_" +
                          std::to_string(tileY - tileSize) + ".bin");

    if (pixelX % tileSize == 0 && pixelY % tileSize == 0 &&
        tileX > 0 && tileY > 0)
    {
        finalTiles.insert("tile_" +
            std::to_string(tileX - tileSize) + "_" +
            std::to_string(tileY - tileSize) + ".bin");
    }
}

    // ---- Final result ----
    std::cout << "\nTiles required for all towers:\n";

    for (const auto& tile : finalTiles)
        std::cout << tile << std::endl;

    return 0;
}