typedef struct {
  int x, y, width, height;
} OtkRect;

PyObject *OtkRect_New(int x, int y, int width, int height);
