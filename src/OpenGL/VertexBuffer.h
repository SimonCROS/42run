//
// Created by loumarti on 1/15/25.
//

#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

class VertexBuffer {
private:
    unsigned int m_rendererID;
public:
    VertexBuffer();
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
    void write(const void* data, unsigned int size);

    [[nodiscard]] unsigned int getID() const { return m_rendererID; }
};

#endif //VERTEXBUFFER_H
