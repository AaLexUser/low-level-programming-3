#include "../src/test.h"
#include "utils/hashtable.h"

DEFINE_TEST(simple){
    hmap_t* hmap = ht_init();
    int num1 = 1;
    int num2 = 2;
    ht_put(hmap, "num1", &num1);
    ht_put(hmap, "num2", &num2);
    int* num1_test = ht_get(hmap, "num1");
    int* num2_test = ht_get(hmap, "num2");
    assert(*num1_test == num1);
    assert(*num2_test == num2);
    ht_free(hmap);
}
int main(){
    RUN_SINGLE_TEST(simple); 
    return 0;
}