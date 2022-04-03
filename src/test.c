#define _DEFAULT_SOURCE

#include <unistd.h>
#include <stdio.h>
#include "mem_internals.h"
#include "mem.h"
#include "util.h"
#include "test.h"

static const size_t INITIAL_HEAP_SIZE = 10000;

void debug(const char *fmt, ...);

static void* test_heap_init(){
	debug("Создаю кучу\n");
	void *heap = heap_init(INITIAL_HEAP_SIZE);
	if (heap == NULL) {
		err("Не получилось создать кучу.");
	}
	debug("Куча успешно создана.\n\n");
	return heap;
}

static void create_new_page(struct block_header *last_block){
	struct block_header *addr = last_block;
	void* test_addr = (uint8_t*) addr + size_from_capacity(addr->capacity).bytes;
	test_addr = mmap( (uint8_t*) (getpagesize() * ((size_t) test_addr / getpagesize() + (((size_t) test_addr % getpagesize()) > 0))), 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,0, 0);
	debug(test_addr);
}

static struct block_header* get_block_from_contents(void * data) {
	return (struct block_header *) ((uint8_t *) data - offsetof(struct block_header, contents));
}

static void test1(struct block_header *first_block) {
	debug("Начинаю 1 тест\n");
	void *data = _malloc(1000);
	if (data == NULL)
		err("Ошибка 1");
	debug_heap(stdout, first_block);
	if (first_block->is_free != false || first_block->capacity.bytes != 1000)
		err("Ошибка 2");
	debug("Тест 1 пройден\n\n");
	_free(data);
}

static void test2(struct block_header *memroy) {
	debug("Начинаю 2 тест\n");
	void *data1 = _malloc(1000);
	void *data2 = _malloc(1000);
	if (data1 == NULL || data2 == NULL)
		err("Ошибка 1");
	_free(data1);
	debug_heap(stdout, memroy);
	struct block_header *data1_block = get_block_from_contents(data1);
	struct block_header *data2_block = get_block_from_contents(data2);
	if (data1_block->is_free == false)
		err("Ошибка 2");
	if (data2_block->is_free == true)
		err("Ошибка 3");
	debug("Тест 2 пройден\n\n");
	_free(data2);
}

static void test3(struct block_header *memory) {
	debug("Начинаю 3 тест\n");
	void *data1 = _malloc(1000);
	void *data2 = _malloc(1000);
	void *data3 = _malloc(1000);
	if (data1 == NULL || data2 == NULL || data3 == NULL)
		err("Ошибка 1");
	_free(data2);
	_free(data1);
	debug_heap(stdout, memory);
	struct block_header *data1_block = get_block_from_contents(data1);
	struct block_header *data3_block = get_block_from_contents(data3);
	if (data1_block->is_free == false)
		err("Ошибка 2");
	if (data3_block->is_free == true)
		err("Ошибка 3");
	if (data1_block->capacity.bytes != 2000 + offsetof(struct block_header, contents))
		err("Ошибка 4");
	debug("Тест 3 пройден\n\n");
	_free(data3);
}

static void test4(struct block_header *memory) {
	debug("Начинаю 4 тест\n");
	void *data1 = _malloc(10000);
	void *data2 = _malloc(10000);
	void *data3 = _malloc(5000);
	if (data1 == NULL || data2 == NULL || data3 == NULL)
		err("Ошибка 1");
	_free(data3);
	_free(data2);
	debug_heap(stdout, memory);
	struct block_header *data1_block = get_block_from_contents(data1);
	struct block_header *data2_block = get_block_from_contents(data2);
	if ((uint8_t *)data1_block->contents + data1_block->capacity.bytes != (uint8_t*) data2_block)
		err("Ошибка 2");
	debug("Тест 4 пройден\n\n");
	_free(data1);
}

static void test5(struct block_header *memory) {
	debug("Начинаю 5 тест\n");
	void *data1 = _malloc(10000);
	if (data1 == NULL)
		err("Ошибка 1");
	struct block_header *addr = memory;
	while (addr->next != NULL) addr = addr->next;
	create_new_page(addr);
	void *data2 = _malloc(100000);
	debug_heap(stdout, memory);
	struct block_header *data2_block = get_block_from_contents(data2);
	if (data2_block == addr)
		err("Ошибка 2");
	debug("Тест 5 пройден\n\n");
	_free(data1);
	_free(data2);
}


void test() {
	struct block_header *memory = (struct block_header*) test_heap_init();
	test1(memory);
	test2(memory);
	test3(memory);
	test4(memory);
	test5(memory);
	debug("Все тесты пройдены\n");
}

