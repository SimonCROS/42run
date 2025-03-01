//
// Created by loumarti on 1/15/25.
//

export module OpenGL:IndicesBuffer;

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
