#include "file.h"
#include "utils/logger.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>

off_t fl_cur_page_offset(file_t* file){
    return file->cur_page_offset;
}

#define fl_max_page_index() (file->file_size / PAGE_SIZE - 1)

void* fl_cur_mmaped_data(file_t* file){
    return file->cur_mmaped_data;
}

uint64_t fl_number_pages(file_t* file){
    return file->file_size / PAGE_SIZE;
}

uint64_t fl_page_index(off_t page_offset){
    return page_offset / PAGE_SIZE;
}

off_t fl_page_offset(uint64_t page_index){
    return (off_t)page_index * PAGE_SIZE;
}

int64_t fl_current_page_index(file_t* file){
    return file->cur_page_offset / PAGE_SIZE;
}

#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>
#if defined(__linux__)
#include <sched.h>
#include <zconf.h>
int ftruncate(int fd, off_t i);

#endif


/**
 * @brief Get the size of a file.
 *
 * This function takes a file_t structure and retrieves the size of the file
 * associated with it. It internally uses the stat() system call to obtain the size.
 *
 * @param file A pointer to a file_t structure representing the file.
 * @return The size of the file in bytes.
 * @note This function assumes that the specified file exists and has valid permissions.
 */
off_t fl_file_size(file_t* file) {
    struct stat st;
    stat(file->filename, &st);
    return st.st_size;
}

/**
 * @brief       File initialization
 * @param[in]   filename: name of file
 * @param[out]  file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int init_file(const char* file_name, file_t* file){
    file->filename = (char*)malloc(strlen(file_name)+1);
    strncpy(file->filename, file_name, strlen(file_name)+1);
    logger(LL_DEBUG, __func__ ,"Opening file %s.", file->filename);
    file->fd = open(file->filename,
                           O_RDWR | // Read/Write mode
                           O_CREAT, // Create file if it does not exist
                           S_IWUSR | // User write permission
                           S_IRUSR // User read permission
    );
    if (file->fd == -1){
        logger(LL_ERROR, __func__ ,"Unable to open file.");
        return FILE_FAIL;
    }
    file->file_size = fl_file_size(file);
    file->max_page_index = fl_max_page_index();
    file->cur_page_offset = 0;
//    if(fl_file_size(file) != 0){
//        if(mmap_page(file->cur_page_offset, file) == FILE_FAIL){
//            logger(LL_ERROR, __func__, "Unable map file");
//            return FILE_FAIL;
//        }
//    }
    return FILE_SUCCESS;
}


/**
 * @brief       Map File
 * @param[in]   offset: offset in file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */
int mmap_page(off_t offset, file_t* file){
    logger(LL_DEBUG, __func__,
           "Mapping page from file with descriptor %d and file size %" PRIu64, file->fd, file->file_size);
    if(file->file_size == 0){
        return FILE_FAIL;
    }
    if((file->cur_mmaped_data = mmap(NULL, PAGE_SIZE,
                                            PROT_WRITE |
                                            PROT_READ,
                           MAP_SHARED, file->fd, offset)) == MAP_FAILED){
        logger(LL_ERROR, __func__ , "Unable to map file: %s %d.", strerror(errno), errno);
        printf("Filesize: %f mb\n", (double)file->file_size / (1024*1024));
        return FILE_FAIL;
    }
    file->cur_page_offset = offset;
    logger(LL_DEBUG, __func__, "page %ld mapped on address %p", fl_current_page_index(file), file->cur_mmaped_data);
    return FILE_SUCCESS;
}

int map_page_on_addr(off_t offset, file_t* file, void* addr){
    logger(LL_DEBUG, __func__,
           "Mapping page from file with descriptor %d and file size %" PRIu64, file->fd, file->file_size);
    if(file->file_size == 0){
        return FILE_FAIL;
    }
    if((file->cur_mmaped_data = mmap(addr, PAGE_SIZE,
                                     PROT_WRITE |
                                     PROT_READ,
                                      MAP_FIXED | MAP_SHARED, file->fd, offset)) == MAP_FAILED){
        logger(LL_ERROR, __func__ , "Unable to map file: %s %d.", strerror(errno), errno);
        printf("Filesize: %f mb\n", (double)file->file_size / (1024*1024));
        return FILE_FAIL;
    }
    file->cur_page_offset = offset;
    logger(LL_DEBUG, __func__, "page %ld mapped on address %p", fl_current_page_index(file), file->cur_mmaped_data);
    return FILE_SUCCESS;
}

/**
 * @brief       Synchronize a mapped region
 * @param[in]   mmaped_data: pointer to mapped data
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int sync_page(void* mmaped_data){
    logger(LL_DEBUG, __func__, "Syncing page on address - %p.", mmaped_data);
    if(msync(mmaped_data, PAGE_SIZE, MS_ASYNC) == -1){
        logger(LL_ERROR, __func__ ,
               "Unable sync page on address - %p: %s %d.", mmaped_data, strerror(errno), errno);
        return FILE_FAIL;
    }
    return FILE_SUCCESS;
}

/**
 * @brief       Unmap page
 * @param[in]   mmaped_data: pointer to mapped data
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int unmap_page(void** mmaped_data, file_t* file){
    if(file->file_size == 0){
        return FILE_SUCCESS;
    }

    logger(LL_DEBUG, __func__,
           "Unmapping page from file with pointer %p and file size %" PRIu64,
           mmaped_data, file->file_size);
    if(sync_page(*mmaped_data) == FILE_FAIL){
        return FILE_FAIL;
    }
    if(munmap(*mmaped_data, PAGE_SIZE) == -1){
        logger(LL_ERROR, __func__, "Unable unmap page with pointer %p: %s %d.", mmaped_data, strerror(errno), errno);
        return FILE_FAIL;
    };
    *mmaped_data = NULL;
    return FILE_SUCCESS;
}

/**
 * @brief       Close file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int close_file(file_t* file){
    close(file->fd);
    file->fd = -1;
    free(file->filename);
    return FILE_SUCCESS;
}

/**
 * @brief       Delete file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int delete_file(file_t* file){
    logger(LL_DEBUG, __func__, "Deleting file with name %s.", file->filename);
    if(unlink(file->filename) == -1){
        logger(LL_ERROR, __func__, "Unable delete file with name %s.", file->filename);
        return FILE_FAIL;
    }
    close_file(file);
    return FILE_SUCCESS;
}

/**
 * @brief       Initialize new page
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int init_page(file_t* file){

    if(!file)
        return FILE_FAIL;

    logger(LL_DEBUG, __func__ , "Init new page");

    off_t file_size = file->file_size;
    if(file_size == -1){
        logger(LL_ERROR, __func__, "Unable to get file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }

    if(ftruncate(file->fd,  (off_t) (file->file_size + PAGE_SIZE)) == -1){
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }
    file->file_size += PAGE_SIZE;
    ++file->max_page_index;

    file->cur_page_offset = file->file_size - PAGE_SIZE;
    if(mmap_page(file->cur_page_offset, file) == FILE_FAIL){
        logger(LL_ERROR, __func__, "Unable to mmap file.");
    }
    return FILE_SUCCESS;
}

/**
 * @brief       Delete last page in file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int delete_last_page(file_t* file){
    if(file->file_size == 0){
        return FILE_SUCCESS;
    }
    logger(LL_DEBUG, __func__ , "Starting delete page %ld", fl_page_index(file->file_size - PAGE_SIZE));
    if(ftruncate(file->fd,  (off_t) (file->file_size - PAGE_SIZE)) == -1){
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }
    file->file_size -= PAGE_SIZE;
    --file->max_page_index;
    return FILE_SUCCESS;
}

/**
 * @brief       Copies size bytes from memory area src to mapped_file_page and make synchronization with
 *              original file.
 * @param[in]   file: pointer to file_t
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int write_page(file_t* file, void* src, uint64_t size, off_t offset){
    logger(LL_DEBUG, __func__ , "Writing to fd %d with file size: %"PRIu64 ", src size: %"PRIu64 " bytes.",
           file->fd, file->file_size, size);
    if(file->cur_mmaped_data == NULL){
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy((uint8_t*)file->cur_mmaped_data + offset, src, size);
    return FILE_SUCCESS;
}

/**
 * @brief       Copies size bytes to memory area dest from mapped_file_page.
 * @param[in]   file: pointer to file_t
 * @param[out]  dest: destination
 * @param[in]   size: size to read
 * @param[in]   offset: offset in page to read from
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int read_page(file_t* file, void* dest, uint64_t size, off_t offset){
    logger(LL_DEBUG, __func__ , "Reading from fd %d with size %"PRIu64 "%"PRIu64 " bytes.",
           file->fd, file->file_size, size);
    if(file->cur_mmaped_data == NULL){
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(dest, (uint8_t*)file->cur_mmaped_data + offset, size);
    return FILE_SUCCESS;
}

#endif
#if defined(_WIN32)
#include <windows.h>

#define geterr(lpMsgBuf) DWORD dwError = GetLastError();\
                LPVOID lpMsgBuf;\
                FormatMessage(\
                FORMAT_MESSAGE_ALLOCATE_BUFFER |\
                FORMAT_MESSAGE_FROM_SYSTEM |\
                FORMAT_MESSAGE_IGNORE_INSERTS,\
                NULL,\
                dwError,\
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),\
                (LPTSTR) &lpMsgBuf,\
                0, NULL )

off_t fl_file_size(file_t* file) {
    LARGE_INTEGER size;
	if (!GetFileSizeEx(file->h_file, &size)) {
		logger(LL_ERROR, __func__, "Unable to get file size.");
		return FILE_FAIL;
	}
	return (off_t)size.QuadPart;
}

static int close_handles(file_t* file){
    if(!CloseHandle(file->h_file)){
        logger(LL_ERROR, __func__, "Cannot close file handle %p", file->h_file);
        return FILE_FAIL;
    }
    if(!CloseHandle(file->h_map)){
        logger(LL_ERROR, __func__, "Cannot close map handle %p", file->h_map);
        return FILE_FAIL;
    };
    return FILE_SUCCESS;
}

static int open_handles(file_t* file){
    file->h_file = CreateFile(file->filename,
                              GENERIC_READ | GENERIC_WRITE, // Read/Write mode
                              0, // No sharing
                              NULL, // Default security attributes
                              OPEN_ALWAYS, // Open existing file or create new file
                              FILE_ATTRIBUTE_NORMAL, // Normal file
                              NULL); // No template file

    if (file->h_file == INVALID_HANDLE_VALUE) {
        logger(LL_ERROR, __func__, "Unable to open file.");
        return FILE_FAIL;
    }
    file->h_map = CreateFileMapping(file->h_file, NULL, PAGE_READWRITE, 0,
                                    0,
                                    NULL);
    if(file->h_map == NULL){
        geterr(lpMsgBuf);
        logger(LL_ERROR, __func__, "Unable to create file mapping: %s.", (char*)lpMsgBuf);
        fflush(stdout);
        if(!CloseHandle(file->h_file)){
            logger(LL_ERROR, __func__, "Cannot close file handle %p", file->h_file);
        }
        return FILE_FAIL;
    }

    return FILE_SUCCESS;
}

/**
 * \brief       File initialization
 * \param[in]   filename: name of file
 * \return      FILE_SUCCESS on success or FILE_FAIL otherwise
 */

int init_file(const char* file_name, file_t* file) {
    file->filename = (char*)malloc(strlen(file_name) + 1);
    strncpy(file->filename,file_name, strlen(file_name) + 1);
    logger(LL_DEBUG, __func__, "Opening file %s.", file->filename);

    file->h_file = CreateFile(file->filename,
        GENERIC_READ | GENERIC_WRITE, // Read/Write mode
        0, // No sharing
        NULL, // Default security attributes
        OPEN_ALWAYS, // Open existing file or create new file
        FILE_ATTRIBUTE_NORMAL, // Normal file
        NULL); // No template file

    if (file->h_file == INVALID_HANDLE_VALUE) {
        logger(LL_ERROR, __func__, "Unable to open file.");
        return FILE_FAIL;
    }
    DWORD low_offset;
    if(fl_file_size(file) == 0){
        low_offset = 20;
    }
    else low_offset = 0;

    file->h_map = CreateFileMapping(file->h_file, NULL, PAGE_READWRITE, 0,
                                    low_offset,//20MB
                                    NULL);
    if(file->h_map == NULL){
        geterr(lpMsgBuf);
        logger(LL_ERROR, __func__, "Unable to create file mapping: %s.", (char*)lpMsgBuf);
        fflush(stdout);
        if(!CloseHandle(file->h_file)){
            logger(LL_ERROR, __func__, "Cannot close file handle %p", file->h_file);
        }
        return FILE_FAIL;
    }
    file->file_size = fl_file_size(file);
    file->max_page_index = fl_max_page_index();
    file->cur_page_offset = 0;
    file->cur_mmaped_data = NULL;

//    SYSTEM_INFO si;
//    GetSystemInfo(&si);
//    DWORD allocation_granularity = si.dwAllocationGranularity;

//    PAGE_SIZE = (off_t)allocation_granularity;


    return FILE_SUCCESS;
}

/**
 * @brief       Map File
 * @param[in]   offset: offset in file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int mmap_page(off_t offset, file_t* file) {
    logger(LL_DEBUG, __func__,
        "Mapping page from file with handel %p and file size %" PRIu64, file->h_file, file->file_size);
    if (file->file_size <= 0) {
        return FILE_FAIL;
    }
    uint64_t offsetu = (uint64_t) offset;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD allocation_granularity = si.dwAllocationGranularity;

    // Ensure offset is a multiple of the allocation granularity
    offsetu = ((offsetu + allocation_granularity - 1) / allocation_granularity) * allocation_granularity;

    DWORD offset_high = (DWORD)((offsetu >> 32) & 0xFFFFFFFFL);
    DWORD offset_low = (DWORD)(offsetu & 0xFFFFFFFFL);

    file->cur_mmaped_data = MapViewOfFile(file->h_map, FILE_MAP_ALL_ACCESS, offset_high, offset_low, PAGE_SIZE);
	if (file->cur_mmaped_data == NULL) {
        geterr(lpMsgBuf);
		logger(LL_ERROR, __func__, "Unable to map file: %s.", (char*) lpMsgBuf);
        fflush(stdout);
		return FILE_FAIL;
	}
	file->cur_page_offset = offset;
	logger(LL_DEBUG, __func__, "page %ld mapped on address %p", fl_current_page_index(file), file->cur_mmaped_data);
	return FILE_SUCCESS;
}

/**
 * @brief       Synchronize a mapped region
 * @param[in]   mmaped_data: pointer to mapped data
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int sync_page(void* mmaped_data) {
    logger(LL_DEBUG, __func__, "Syncing page on address - %p.", mmaped_data);
    if (!FlushViewOfFile(mmaped_data, PAGE_SIZE)) {
        geterr(lpMsgBuf);
        logger(LL_ERROR, __func__, "Unable sync page: %s.", (char*) lpMsgBuf);
        fflush(stdout);
        return FILE_FAIL;
    }
    return FILE_SUCCESS;
}


/**
 * @brief       Unmap page
 * @param[in]   mmaped_data: pointer to mapped data
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int unmap_page(void** mmaped_data, file_t* file) {
    if (file->file_size == 0) {
        return FILE_SUCCESS;
    }

    logger(LL_DEBUG, __func__,
        "Unmapping page from file with pointer %p and file size %" PRIu64,
        *mmaped_data, file->file_size);
    if (sync_page(*mmaped_data) == FILE_FAIL) {
        return FILE_FAIL;
	}
    if (!UnmapViewOfFile(*mmaped_data)) {
		logger(LL_ERROR, __func__, "Unable unmap page with pointer %p: %s %d.", *mmaped_data, strerror(errno), errno);
		return FILE_FAIL;
	};
    *mmaped_data = NULL;
	return FILE_SUCCESS;
}

/**
 * @brief       Close file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int close_file(file_t* file) {
    if(file->cur_mmaped_data != NULL){
        unmap_page(&file->cur_mmaped_data, file);
    }
    if(!CloseHandle(file->h_file)){
        logger(LL_ERROR, __func__, "Cannot close file handle %p", file->h_file);
    }
    if(!CloseHandle(file->h_map)){
        logger(LL_ERROR, __func__, "Cannot close map handle %p", file->h_map);
    };
	file->h_file = INVALID_HANDLE_VALUE;
    file->h_map = INVALID_HANDLE_VALUE;
	free(file->filename);
	return FILE_SUCCESS;
}

/**
 * @brief       Delete file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int delete_file(file_t* file) {
    logger(LL_DEBUG, __func__, "Deleting file with name %s.", file->filename);
    char*  filename = malloc(strlen(file->filename)+1);
    strncpy(filename, file->filename, strlen(file->filename)+1);
    close_file(file);
	if (DeleteFile(filename) == 0) {
        geterr(lpMsgBuf);
        logger(LL_ERROR, __func__, "Unable to delete file with name %s. Error: %s", filename, (char*)lpMsgBuf);
        fflush(stdout);
        LocalFree(lpMsgBuf);
        free(filename);
        return FILE_FAIL;
	}
	free(filename);
	return FILE_SUCCESS;
}

/**
 * @brief       Initialize new page
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int init_page(file_t* file) {
    if (SetFilePointer(file->h_file, (off_t)(file->file_size + PAGE_SIZE), NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }

    if (!SetEndOfFile(file->h_file)) {
        logger(LL_ERROR, __func__, "Unable change end of file: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }

    file->cur_page_offset = file->file_size;
    file->file_size += PAGE_SIZE;
    file->max_page_index++;

    if(close_handles(file) == FILE_FAIL) {return FILE_FAIL;}
    if(open_handles(file) == FILE_FAIL) {return FILE_FAIL;}

    if (mmap_page(file->cur_page_offset, file) == FILE_FAIL) {
        logger(LL_ERROR, __func__, "Unable to mmap file.");
        return FILE_FAIL;
    }
    return FILE_SUCCESS;
}

/**
 * @brief       Delete last page in file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int delete_last_page(file_t* file) {
    if (file->file_size == 0) {
        return FILE_SUCCESS;
    }
    if(!CloseHandle(file->h_map)){
        logger(LL_ERROR, __func__, "Cannot close map handle %p", file->h_map);
    };
    logger(LL_DEBUG, __func__, "Starting delete page %ld", fl_page_index(file->file_size - PAGE_SIZE));
    if (SetFilePointer(file->h_file, (off_t)(file->file_size - PAGE_SIZE), NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        geterr(lpMsgBuf);
        logger(LL_ERROR, __func__, "Unable change file size: %s", (char*)lpMsgBuf);
        fflush(stdout);
        LocalFree(lpMsgBuf);
        return FILE_FAIL;
    }
    if (!SetEndOfFile(file->h_file)) {
        geterr(lpMsgBuf);
        logger(LL_ERROR, __func__, "Unable change end of file. Error: %s", (char*)lpMsgBuf);
        fflush(stdout);
        LocalFree(lpMsgBuf);
        return FILE_FAIL;
    }
    file->file_size -= PAGE_SIZE;
    file->max_page_index--;
    file->h_map = CreateFileMapping(file->h_file, NULL, PAGE_READWRITE, 0,
                                    0,
                                    NULL);
    return FILE_SUCCESS;
}

/**
 * @brief       Copies size bytes from memory area src to mapped_file_page and make synchronization with
 *              original file.
 * @param[in]   file: pointer to file_t
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int write_page(file_t* file, void* src, uint64_t size, off_t offset) {
    logger(LL_DEBUG, __func__, "Writing to handle %p with size %" PRIu64 "%" PRIu64 " bytes.",
        file->h_file, file->file_size, size);
    if (file->cur_mmaped_data == NULL) {
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(file->cur_mmaped_data + offset, src, size);
    return FILE_SUCCESS;
}

/**
 * @brief       Copies size bytes to memory area dest from mapped_file_page.
 * @param[in]   file: pointer to file_t
 * @param[out]  dest: destination
 * @param[in]   size: size to read
 * @param[in]   offset: offset in page to read from
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int read_page(file_t* file, void* dest, uint64_t size, off_t offset) {
    logger(LL_DEBUG, __func__, "Reading from handle %p with size %" PRIu64 "%" PRIu64 " bytes.",
        file->h_file, file->file_size, size);
    if (file->cur_mmaped_data == NULL) {
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(dest, file->cur_mmaped_data + offset, size);
    return FILE_SUCCESS;
}
#endif

