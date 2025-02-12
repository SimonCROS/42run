//
// Created by loumarti on 1/15/25.
//

#ifndef INDICESBUFFER_H
#define INDICESBUFFER_H

class IndicesBuffer {
private:
    unsigned int m_rendererID;
public:
    IndicesBuffer();
    ~IndicesBuffer();

    void bind() const;
    void unbind() const;
    void write(const void* data, unsigned int size);

    [[nodiscard]] unsigned int getID() const { return m_rendererID; }
};

#endif //INDICESBUFFER_H
