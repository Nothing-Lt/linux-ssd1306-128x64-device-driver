#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/of_device.h>
#include <linux/i2c.h>

#include "lib_ssd1306.h"
#include "ssd1306_fonts.h"

/*
    //////////////////////////////////////////////////////////////////
    //                      i2c bus core                            //
    //////////////////////////////////////////////////////////////////
                   ||                                   ||
        /////////////////////////             /////////////////////////
        // (1) i2c device list //             // (2) i2c driver list //
        /////////////////////////             /////////////////////////
    
    The i2c mechanism in linux kernel has seperated 2 parts.
    They are (1)i2c device list and (2)i2c driver list.
    If you want to implement a i2c device driver for an i2c sensor,
    then these 2 parts you have to finish.
    
    (1) i2c device list. This is a list for the i2c device, any i2c sensor should be add to this list.
        For adding the device to this list, we can use device tree overlay.
        The implemented device tree is as follow:
        /dts-v1/;
        /plugin/;

        /{ // this is our device tree overlay root node

          compatible = "ti,beaglebone", "ti,beaglebone-black";
          part-number = "BBB-I2C2"; // you can choose any name here but it should be memorable
          version = "00A0";
        
          fragment@0 { // fragment keyword is very important.
            target = <&i2c2>; // Target is the target bus we will add this device in.
            __overlay__ { // This __overlay__ keyword is very important.
              SSD1306: SSD1306@3C { // the SSD1306 defined as child of the i2c2 bus
                compatible = "SSD1306"; // This property should be same as the .compatible element in board_info in ssd1306 driver.
                #address-cells = <1>;
                #size-cells = <0>;
                reg = <0x3C>; // Declare the i2c address of this device
              };
            };
          };
        }; // root node end 

    a. Compile with command line "dtc -O dtb -o SSD1306-00A0.dtbo -b 0 -@ SSD1306.dts"  
    b. put the .dtbo file to /lib/firmware
    c. modify the uEnv.txt file and let the kernel load this .dtbo file

    (2) i2c driver list. This is a list for i2c driver, any i2c device driver should be add to this list.
        For adding driver to this list, we can do list this
        
        static const struct i2c_device_id ssd1306_ids[] = {
            {"SSD1306", 0},// This name should be same as the name in SSD1306_board_info
            {}
        };
        MODULE_DEVICE_TABLE(i2c, ssd1306_ids);

        static struct i2c_driver ssd1306_driver = {
            .driver = {
                .name = "SSD1306",
                .owner = THIS_MODULE,
            },

            .probe_new = ssd1306_probe,
            .remove = ssd1306_remove,
            .id_table = ssd1306_ids,
        };
        
        static int __init ssd1306_module_init(void)
        {
            i2c_add_driver(&ssd1306_driver);
            return 0;
        }
        module_init(ssd1306_module_init);

        a. define a i2c_device_id variable,
           i2c_device_id is the device id which supported by this driver.
        b. use i2c_driver to announce a i2c driver structure variable.
        c. call the i2c_add_driver and passing the announced i2c_driver.

    When the developer calling the function i2c_new_device or i2c_add_driver, 
    the i2c bus core will compare the element in these two list.
    The compare will be based on the name in i2c_device_id of the driver element 
    and name in i2c_board_info of the device element.
    If they are matching, then the probe function will be calling to run,
    then the register_chrdev will be called in probe function. 

    For implementing a i2c device driver, these 2 steps is must.
    and in some of implmentations. They seperated the device and driver into 
    different files. But this driver demo shown me that write all of the things into same file
    is also working.
*/

int major;
static struct i2c_adapter* ssd1306_adapter = NULL;
struct        i2c_client*  ssd1306_i2c_client = NULL;
static struct class*       cl = NULL;
static struct device*      dev = NULL;

uint8_t c_buf[4096];

static int ssd1306_write(struct file* fp, const char* data, size_t size, loff_t* offset)
{
    copy_from_user(c_buf, data, size);
    SSD1306_print(c_buf, Font_7x10, SSD1306_WHITE);
    SSD1306_refresh();

    return 0;
}

static loff_t ssd1306_lseek(struct file* file, loff_t offset, int orig)
{
    SSD1306_cursor_set((offset>>8) & 0xFF,offset & 0xFF);
    return 0;
}

static struct file_operations file_ops = {
    .write = ssd1306_write,
    .llseek = ssd1306_lseek
};

static const struct of_device_id ssd1306_of_match[] = {
    { .compatible = "SSD1306"},
    {}
};
MODULE_DEVICE_TABLE(of, ssd1306_of_match);

static int ssd1306_probe(struct i2c_client* client)
{
    printk(KERN_INFO "[SSD1306] start probe \n");

    ssd1306_i2c_client = client;
    
    SSD1306_init();
    SSD1306_print("Hello this is ssd1306 i2c module", Font_7x10, SSD1306_WHITE);
    SSD1306_refresh();
    SSD1306_cursor_set(0, 0);

    //Add this driver to /dev
    major = register_chrdev(0, "SSD1306", &file_ops);
    if(0 > major){
        printk(KERN_INFO "[SSD1306] major failed\n");
        goto label_major_failed;
    }

    cl = class_create(THIS_MODULE, "SSD1306_CL");
    if(IS_ERR(cl)){
        printk(KERN_INFO "[SSD1306] class failed\n");
        goto label_class_failed;
    }
    
    dev = device_create(cl, NULL, MKDEV(major,0), NULL, "SSD1306");        
    if(IS_ERR(dev)){
        goto label_device_failed;
    }

    return 0;

label_device_failed:
    class_unregister(cl);
    class_destroy(cl);    
label_class_failed:
    unregister_chrdev(major,"SSD1306");
label_major_failed:
    printk(KERN_INFO "[SSD1306] probe failed \n");
    return 0;
}

static int ssd1306_remove(struct i2c_client* client)
{
    // Remove this driver from /dev
    device_destroy(cl,MKDEV(major,0));
    class_unregister(cl);
    class_destroy(cl);
    unregister_chrdev(major,"SSD1306");
    return 0;
}

static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = "SSD1306",
        .owner = THIS_MODULE,
        .of_match_table = ssd1306_of_match,
    },

    .probe_new = ssd1306_probe,
    .remove = ssd1306_remove,
};

static int __init ssd1306_module_init(void)
{
    // This part corespond for the driver part.
    printk(KERN_INFO "[SSD1306] Adding driver to driver list\n");
    i2c_add_driver(&ssd1306_driver);
    
    return 0;
}
module_init(ssd1306_module_init);

static void __exit ssd1306_module_exit(void)
{
    // Remving from device list
    i2c_unregister_device(ssd1306_i2c_client);

    // Remving from driver list
    i2c_del_driver(&ssd1306_driver);
}
module_exit(ssd1306_module_exit);

MODULE_LICENSE("GPL");