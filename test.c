/* Small Test for your first module driver draft 0.01*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "Epitech_ioctl.h"

#define USER_BUFFER_LEN 1024*1024*3 /* Your driver has the same space */ 


#define DEVICE_NAME "/dev/Epitech_example"

#define PATTERN_PATH "./PATTERN.jpg"

/* These tests are executed as follow
 * First a Consumer process is launched
 * then a Producer process is launched
 */

#define TEST_1 /* from user space to kernel space */
#define TEST_2 /* garding your kernel memory space from wrong users */
#define TEST_3 /* mapping user space to kernel space memory  transfering data from Producer to consumer*/
#define TEST_4 /* Same operation the other way around */

/* WIP from my side at the moment. Make the driver and the test thread safe. 
 * Exectue X Producers and Y Consumers at the same time. Starting and finishing the tests
 * at the same time.
 * You launch X Producer and Y Consumer.
 * You raise a barrier to execute all the test at the same time. Synching at each steps via the module
 */
#define TEST_5 

/* pointer shared between kernel space and user space*/
static char * shared_map = NULL;

static char Producer_array[USER_BUFFER_LEN] = {'0'};   
static char Consumer_array[USER_BUFFER_LEN] = {'0'}; 


static char * PATTERN = NULL; /* pointer storing the pattern*/

static int PATTERN_LEN = 0;

/* add new ioctls. The module will take charge of synching all the processes */
#ifdef TEST_5
static int Consumer_Thread_Safe();
static int Prodcuer_Thread_Safe();
#endif

static int Consumer(int test)
{
	int c_fd;
	int pattern_fd;

	printf("Launching Consumer...\n");

	/* MEM INIT */
	if ( (PATTERN = (char *)malloc(USER_BUFFER_LEN)) == NULL)
	{
		printf("Failed to alloc Consumer Buffer\n");
		return -1;
	}

	/* PATTERN INIT */
	if( 0 > (pattern_fd = open(PATTERN_PATH , O_RDONLY)) )
	{
		perror("failed to open the device...");
		return errno;
	}

	if ( 0 >= (PATTERN_LEN = read(pattern_fd, PATTERN , USER_BUFFER_LEN)) )
	{
		perror("failed to load PATTERN");
		return errno;
	}
	else	
	{
		printf("Pattern load successfully patern length  %d\n",PATTERN_LEN);
	}

	close(pattern_fd);

	/* TEST BEGIN */
	if( (c_fd = open(DEVICE_NAME, O_RDWR)) < 0)
	{
		perror("failed to open the device...");
		return errno;
	}

	/* We clear the kernel buffer */
	if( 0 !=  ioctl(c_fd, EPITECH_DRV_CLEAR_BUFFER))
	{
		printf("IOCTL unexpected return value \n");
		return -EXIT_FAILURE;
	}


#ifdef TEST_1
	if(test == 1)
	{
		printf("Launching Consumer...TEST 1\n");

		/* The Consumer will sleep waiting for the writer */
		if( (read(c_fd, Consumer_array, USER_BUFFER_LEN)) != USER_BUFFER_LEN )
		{
			perror("failed to Read from DRV");
			return errno;
		}

		/* We test if we recieved the pattern */
		if ( memcmp( Consumer_array, PATTERN, USER_BUFFER_LEN )  != 0)
		{
			printf("Pattern memcmp failed \n");
			return -EXIT_FAILURE;
		}
		else	
		{
			printf("Consumer...TEST 1 OK\n");
		}

		goto  end;
	}
#endif

#ifdef TEST_2
	if(test == 2)
	{
		printf("Launching Consumer...TEST 2\n");

		/* This call should fail with -EINVAL
		 * We are exceeding The kernel buffer space 
		 */ 
		shared_map = mmap(NULL, USER_BUFFER_LEN + 10, PROT_WRITE, MAP_SHARED, c_fd ,0);

		/* my mahcine is 64 bits */
		if((uint64_t) shared_map == -EXIT_FAILURE )
		{
			if(errno != EINVAL)
			{
				printf("mmap wrong parameter test failed, %d\n",errno);
				return -EXIT_FAILURE;
			}
			else
			{
				printf("Consumer...TEST 2 OK\n");
			}
		}

		goto  end;
	}
#endif


#ifdef TEST_3
	if(test == 3)
	{
		printf("Launching Consumer...TEST 3\n");

		/* We map KERNEL_SPACE address to USER_SPACE */
		shared_map = mmap(NULL, USER_BUFFER_LEN, PROT_READ | PROT_WRITE,  MAP_SHARED, c_fd ,0);

		if(shared_map == MAP_FAILED)
		{
			perror("mmap");
			return -EXIT_FAILURE;
		}

		printf("Consumer... Wait for Producer\n");
		if( 0 !=  ioctl(c_fd, EPITECH_DRV_SYNC_BARRIER))
		{
			printf("IOCTL unexpected return value \n");
			return -EXIT_FAILURE;
		}

		if ( memcmp( shared_map, PATTERN, USER_BUFFER_LEN )  != 0)
		{
			printf("Pattern memcmp failed \n");
			return -EXIT_FAILURE;
		}
		else
		{
			printf("Consumer...TEST 3 OK\n");
		}
		goto  end;
	}

#endif

#ifdef TEST_4

	if(test == 4)
	{
		printf("Launching Consumer...TEST 4\n");

		// We map KERNEL_SPACE address to USER_SPACE 
		shared_map = mmap(NULL, USER_BUFFER_LEN, PROT_READ| PROT_WRITE, MAP_SHARED, c_fd ,0);
		if(shared_map == MAP_FAILED)
		{
			perror("mmap");
			return -EXIT_FAILURE;
		}

		//We check the shared space is blank
		for (int i = 0; i< USER_BUFFER_LEN  ;i++)
		{
			if(shared_map[i] != 0)
			{
				printf("Consumer...TEST 4 initial check fail KO\n");
				return -EXIT_FAILURE;
			}
		}

		//We update it with 0xff
		for (int i = 0; i< USER_BUFFER_LEN  ;i++)
		{
			shared_map[i] = 1;
		}

		msync(shared_map,USER_BUFFER_LEN,MS_SYNC);

		printf("Consumer... Waking up the Producer\n");
		if( 0 !=  ioctl(c_fd, EPITECH_DRV_SYNC_BARRIER))
		{
			printf("IOCTL unexpected return value \n");
			return -EXIT_FAILURE;
		}
		
		sleep(1);

		goto  end;
	}

#endif

end :
	/* TEST END */
	close(c_fd);
	free(PATTERN);

	printf("terminating Consumer...\n");

	return EXIT_SUCCESS;

}

static int Producer(int test)
{
	int p_fd;
	int pattern_fd;

	printf("Launching Producer...\n");

	/* MEM INIT */

	if ( (PATTERN = (char *)malloc(USER_BUFFER_LEN)) == NULL)
	{
		printf("Failed to alloc Producer Buffer\n");
		return -1;
	}

	for (int i = 0; i< USER_BUFFER_LEN; i++)
		PATTERN[i] = 0; 

	/* PATTERN INIT */
	if( (pattern_fd = open(PATTERN_PATH , O_RDONLY)) < 0)
	{
		perror("failed to open the device...");
		return errno;
	}

	if ( 0 >= (PATTERN_LEN = read(pattern_fd, PATTERN, USER_BUFFER_LEN)) )
	{
		perror("failed to load PATTERN");
		return errno;
	}
	else	
	{
		printf("Pattern load successfully pattern_len  %d bytes \n",PATTERN_LEN);
	}

	close(pattern_fd);

	/* Producer Buffer setup */
	memcpy(Producer_array, PATTERN,sizeof(Producer_array));

	/* PATTERN <-> PRODUCER BUFFER CHECK */

	if ( memcmp( Producer_array, PATTERN, USER_BUFFER_LEN )  != 0)
	{
		printf("Pattern memcmp failed \n");
		return -EXIT_FAILURE;
	}


	/* TEST BEGIN */
	if( (p_fd = open(DEVICE_NAME, O_RDWR)) < 0)
	{
		perror("failed to open the device...");
		return errno;
	}

#ifdef TEST_1
	if(test == 1)
	{
		printf("Launching Producer...TEST 1\n");
		/* The consumer sleeps waiting for the Producer */
		if( (write(p_fd, Producer_array, USER_BUFFER_LEN)) != USER_BUFFER_LEN )
		{
			perror("failed to Write to DRV");
			return errno;
		}
		goto end;
	}

#endif


#ifdef TEST_3
	if(test == 3)
	{
		printf("Launching Producer...TEST 3\n");

		// We map KERNEL_SPACE address to USER_SPACE 
		shared_map = mmap(NULL, USER_BUFFER_LEN, PROT_READ| PROT_WRITE, MAP_SHARED, p_fd ,0);
		if(shared_map == MAP_FAILED)
		{
			perror("mmap");
			return -EXIT_FAILURE;
		}

		//We write the pattern through shared memory access
		memcpy(shared_map, PATTERN, USER_BUFFER_LEN );

		printf("Producer... Waking up for Consumer\n");
		if( 0 !=  ioctl(p_fd, EPITECH_DRV_SYNC_BARRIER))
		{
			printf("IOCTL unexpected return value \n");
			return -EXIT_FAILURE;
		}

		goto end;
	}
#endif

#ifdef TEST_4
	if(test == 4)
	{
		printf("Launching Producer... TEST 4\n");

		// We map KERNEL_SPACE address to USER_SPACE 
		shared_map = mmap(NULL, USER_BUFFER_LEN, PROT_READ| PROT_WRITE, MAP_SHARED, p_fd ,0);
		if(shared_map == MAP_FAILED)
		{
			perror("mmap");
			return -EXIT_FAILURE;
		}

		printf("Producer... Waits for Consumer\n");
		if( 0 !=  ioctl(p_fd, EPITECH_DRV_SYNC_BARRIER))
		{
			printf("IOCTL unexpected return value \n");
			return -EXIT_FAILURE;
		}

		//We update the shared space 
		for (int i = 0; i< USER_BUFFER_LEN  ;i++)
		{
			if(shared_map[i] != 1)
			{
				printf("Producer...TEST 4 : shared_map[%d] = %d KO\n",i,shared_map[i]);
				return -EXIT_FAILURE;
			}
		}

		printf("Consumer...TEST 4 OK\n");
		goto  end;
	}

#endif


	/* TEST END */
end:
	close(p_fd);
	free(PATTERN);

	printf("terminating Producer...\n");

	return EXIT_SUCCESS;
}


int main (int argc, char ** argv)
{
	printf("TEST EPITECH DRV\n");

	if(argc != 3)
	{
help:
		printf("test 0 1-> launches a Consumer and test 1\n");
		printf("test 1 1-> launches a Producer and test 1\n");

		return -EXIT_FAILURE;
	}

	if( atoi(argv[1]) == 1 ) 
	{
		Producer(atoi(argv[2]));
	}
	else if ( atoi(argv[1]) == 0)
	{
		Consumer(atoi(argv[2]));
	}
	else
	{
		goto help;
	}

	return EXIT_SUCCESS;
}
