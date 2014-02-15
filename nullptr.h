#ifndef NULLPTR_H
#define NULLPTR_H
const class nullptr_t
{
public:
    template<class T>
    operator T*() const { return 0; }

    template<class C, class T>
    operator T C::*() const { return 0; }

private:
    void operator&() const;
} nullptr = {};
#endif // NULLPTR_H