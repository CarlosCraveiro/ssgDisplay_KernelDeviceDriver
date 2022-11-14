#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/proc_fs.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CarlosCraveiro");
MODULE_DESCRIPTION("A simple LKM created to be a device driver to operate a seven segment display");

/********Declaração das funções dos arquivos********/ 
static ssize_t show_value( struct class *class, struct class_attribute *attr, char *buf ); 
static ssize_t store_decimal_value( struct class *class, struct class_attribute *attr, const char *buf, size_t count ); 
static ssize_t store_buffer_value( struct class *class, struct class_attribute *attr, const char *buf, size_t count );
void write_decimal_to_display(long value, int error);
void write_buffer_to_display(unsigned int buffer);

 /***************Variaveis Globais*****************/
volatile long display_value;

/********Funções de escrita e leitura arquivo value*********/
static ssize_t show_value( struct class *class, struct class_attribute *attr, char *buf ) {

      return sprintf(buf, "%ld", display_value);
}

static ssize_t store_decimal_value( struct class *class, struct class_attribute *attr, const char *user_buffer, size_t count ) { 
	char number_buffer[30];
	int error = 0;
	long number = 0;

	strncpy(number_buffer, user_buffer, count - 1);
	number_buffer[count - 1] = '\0';
	
	error = kstrtol(number_buffer, 10, &number);	

	//DEBUG
	printk("dt-gpio - value = %ld \n", number);
	printk("dt-gpio - error flag = %d \n", error);
	printk("dt-gpio - number_buffer = %s \n", number_buffer);
	
	display_value = number;

	write_decimal_to_display(number, error);

       	return count;
}

static ssize_t store_buffer_value( struct class *class, struct class_attribute *attr, const char *user_buffer, size_t count ) {
	char number_buffer[30];
	int error = 0;
	long bin_integer, number;

	strncpy(number_buffer, user_buffer, count - 1);
	number_buffer[count - 1] = '\0';
	
	error = kstrtol(number_buffer, 2, &bin_integer);	
	error = kstrtol(number_buffer, 10, &number);
	
	//DEBUG
	printk("dt-gpio - value = %ld \n", bin_integer);
	printk("dt-gpio - error flag = %d \n", error);
	printk("dt-gpio - number_buffer = %s \n", number_buffer);
	
	display_value = number;

	write_buffer_to_display(bin_integer);
	return count;
}

/* Declate the probe and remove functions */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "7_segment_display",
	}, { /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
	.probe = dt_probe,
	.remove = dt_remove,
	.driver = {
		.name = "my_device_driver",
		.of_match_table = my_driver_ids,
	},
};

static struct class device_class = {
	.name = "7segment_display",
     	.owner = THIS_MODULE
};

struct class_attribute attr_decimal = {
	.show = show_value,
	.store = store_decimal_value,
	.attr.name = "decimal_value",
	.attr.mode = 0777
};

struct class_attribute attr_buffer = {
	.show = show_value,
	.store = store_buffer_value,
	.attr.name = "buffer",
	.attr.mode = 0777
};

/* GPIO variable */
static struct gpio_desc *my_segment_a = NULL;
static struct gpio_desc *my_segment_b = NULL;
static struct gpio_desc *my_segment_c = NULL;
static struct gpio_desc *my_segment_d = NULL;
static struct gpio_desc *my_segment_e = NULL;
static struct gpio_desc *my_segment_f = NULL;
static struct gpio_desc *my_segment_g = NULL;
static struct gpio_desc *my_segment_dp = NULL;

static struct gpio_desc *my_display[8];

void write_buffer_to_display(unsigned int buffer) {
	union {
		struct {
			unsigned int dp : 1;
			unsigned int g : 1;
			unsigned int f : 1;
			unsigned int e : 1;
			unsigned int d : 1;
			unsigned int c : 1;
			unsigned int b : 1;
			unsigned int a : 1;
		} bits;

		unsigned int buffer;
	} segments;
	
	segments.buffer = buffer;

	gpiod_set_value(my_display[0], segments.bits.a);
	gpiod_set_value(my_display[1], segments.bits.b);
	gpiod_set_value(my_display[2], segments.bits.c);
	gpiod_set_value(my_display[3], segments.bits.d);
	gpiod_set_value(my_display[4], segments.bits.e);
	gpiod_set_value(my_display[5], segments.bits.f);
	gpiod_set_value(my_display[6], segments.bits.g);
	gpiod_set_value(my_display[7], segments.bits.dp);
}

void write_decimal_to_display(long value, int error) {
	unsigned int buffer;
	
	if(error == 0) {
		switch(value) {
			case 0:
				buffer = 252;
				break;	
			case 1:
				buffer = 96;
				break;
			case 2:
				buffer = 218;
				break;
			case 3:
				buffer = 242;
				break;
			case 4:
				buffer = 102;
				break;
			case 5:
				buffer = 182;
				break;
			case 6:
				buffer = 190;
				break;
			case 7:
				buffer = 224;
				break;
			case 8:
				buffer = 254; // 8'b11111110
				break;
			case 9:
				buffer = 246;
				break;
			case 10:
				buffer = 238;
				break;
			case 11:
				buffer = 62;
				break;
			case 12:
				buffer = 156;
				break;
			case 13:
				buffer = 122;
				break;
			case 14:
				buffer = 158; //'E' 8'b10011110
				break;
			case 15:
				buffer = 142; //'F' 8'b10001110
				break;
			default:
				buffer = 58; //'o' out-of-range 8'b00111010
		}
	} else {
		buffer = 10; // 'r' from error 8'b00001010
	}

	write_buffer_to_display(buffer);	
}

static int dt_probe(struct platform_device *pdev) {
	struct device *dev = &pdev->dev;
	const char *status; 
	int ret, i;

	printk("dt_gpio - Now I am in the probe function!\n");

	class_register(&device_class);

	my_display[0] = my_segment_a;
	my_display[1] = my_segment_b;
	my_display[2] = my_segment_c;
	my_display[3] = my_segment_d;
	my_display[4] = my_segment_e;
	my_display[5] = my_segment_f;
	my_display[6] = my_segment_g;
	my_display[7] = my_segment_dp;

	ret = class_create_file(&device_class, &attr_decimal);
	if(ret != 0) {
		printk("dt_gpio - Error! Not possible to create class file!\n");
		return -1;	
	}

	ret = class_create_file(&device_class, &attr_buffer);
	if(ret != 0) {
		printk("dt_gpio - Error! Not possible to create class file!\n");
		return -1;	
	}

	if(!device_property_present(dev, "a-gpio")) {
		printk("dt_gpio - Error! Device property 'a-gpio' not found!\n");
		return -1;
	}

	if(!device_property_present(dev, "b-gpio")) {
		printk("dt_gpio - Error! Device property 'b-gpio' not found!\n");
		return -1;
	}

	if(!device_property_present(dev, "c-gpio")) {
		printk("dt_gpio - Error! Device property 'c-gpio' not found!\n");
		return -1;
	}

	/* Read device properties */
	ret = device_property_read_string(dev, "status", &status);
	if(ret) {
		printk("dt_gpio - Error! Could not read 'label'\n");
		return -1;
	}
	
	printk("dt_gpio - status: %s\n", status);
	
	/* Init GPIO */
	my_display[0] = gpiod_get(dev, "a", GPIOD_OUT_LOW);
	my_display[1] = gpiod_get(dev, "b", GPIOD_OUT_LOW);
	my_display[2] = gpiod_get(dev, "c", GPIOD_OUT_LOW);
	my_display[3] = gpiod_get(dev, "d", GPIOD_OUT_LOW);
	my_display[4] = gpiod_get(dev, "e", GPIOD_OUT_LOW);
	my_display[5] = gpiod_get(dev, "f", GPIOD_OUT_LOW);
	my_display[6] = gpiod_get(dev, "g", GPIOD_OUT_LOW);
	my_display[7] = gpiod_get(dev, "dp", GPIOD_OUT_LOW);

	for(i = 0; i < 8; i++) {
		if(IS_ERR(my_display[i])) {
			printk("dt_gpio - Error! Could not setup the GPIO pin %d\n", i);
			return -1 * IS_ERR(my_display[i]);
		}		
	}

	return 0;
}

/**
 * @brief This function is called on unloading the driver 
 */
static int dt_remove(struct platform_device *pdev) {
	int i;
	printk("dt_gpio - Now I am in the remove function\n");	
	
	for(i = 0; i < 8; i++) {
		gpiod_put(my_display[i]);
	}

	return 0;
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("dt_gpio - Loading the driver...\n");
	if(platform_driver_register(&my_driver)) {
		printk("dt_gpio - Error! Could not load driver\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("dt_gpio - Unload driver");
	class_unregister(&device_class);
	platform_driver_unregister(&my_driver); 
}

module_init(my_init);
module_exit(my_exit);
