#include "texture.h"

Texture::Texture(std::string filename) {
    m_filename = filename;

    // load texture img once into memory
    const QString file = QString::fromStdString(filename);
    QImage myImage;
    myImage.load(file);
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_width = myImage.width();
    m_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_imgData.clear();
    m_imgData.reserve(m_width * m_height);
    for (int i = 0; i < arr.size() / 4.f; i++) {
        m_imgData.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
}

std::string Texture::getFilename() {
    return m_filename;
};

RGBA Texture::getTextureColorAtUV(glm::vec2 UV, int repeatU, int repeatV) {
    auto [row, col] = UVtoImgCoord(UV, repeatU, repeatV);
    return m_imgData[row*m_width + col];
}


std::tuple<int, int> Texture::UVtoImgCoord(glm::vec2 UV, int repeatU, int repeatV) {
    float U = UV[0]; 
    float V = UV[1]; 
    int height = m_height;
    int width = m_width;

    float row = std::min(
        (int) floor((1-V) * repeatV * height) % height, // handles repeated textures
        (height - 1) // clip index to [0, h-1]
    ); 
    float col = std::min(
        (int) floor(U * repeatU * width) % width,
        (width - 1)
    ); 

    // assume (0,0) pixel is at top left of image
    return std::make_tuple<int, int>(row, col);
}
