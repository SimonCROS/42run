#ifndef MODEL_H
#define MODEL_H

class Model
{
public:
    void draw();

    static bool loadAscii(const char *filename, Model &model);
    static bool loadBinary(const char *filename, Model &model);
private:
    static bool load(const char *filename, Model &model, bool binary);
};

#endif
