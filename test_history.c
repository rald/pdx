#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "history.h"

void test_history() {
    size_t size = 4;
    byte buffer[4] = {0, 0, 0, 0};
    History* h = History_New(size, 2);

    /* Push 1 */
    History_Push(h, buffer);
    buffer[0] = 1;

    /* Push 2 */
    History_Push(h, buffer);
    buffer[0] = 2;

    /* Undo to state 1 */
    assert(History_Undo(h, buffer) == 1);
    assert(buffer[0] == 1);

    /* Undo to state 0 */
    assert(History_Undo(h, buffer) == 1);
    assert(buffer[0] == 0);

    /* No more undo */
    assert(History_Undo(h, buffer) == 0);

    /* Redo to state 1 */
    assert(History_Redo(h, buffer) == 1);
    assert(buffer[0] == 1);

    /* Redo to state 2 */
    assert(History_Redo(h, buffer) == 1);
    assert(buffer[0] == 2);

    /* No more redo */
    assert(History_Redo(h, buffer) == 0);

    /* Push 3 should clear redo */
    buffer[0] = 3;
    History_Push(h, buffer);
    assert(History_Redo(h, buffer) == 0);

    History_Free(h);
    printf("test_history passed!\n");
}

int main() {
    test_history();
    return 0;
}
