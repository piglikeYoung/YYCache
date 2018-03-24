//
//  YYKVStorage.h
//  YYCache <https://github.com/ibireme/YYCache>
//
//  Created by ibireme on 15/4/22.
//  Copyright (c) 2015 ibireme.
//
//  This source code is licensed under the MIT-style license found in the
//  LICENSE file in the root directory of this source tree.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 YYKVStorageItem is used by `YYKVStorage` to store key-value pair and meta data.
 Typically, you should not use this class directly.
 
 YYKVStorageItem 用来存储键值对数据及拓展数据，通常不应该直接只用它
 */
@interface YYKVStorageItem : NSObject

/**
 消息key值
 */
@property (nonatomic, strong) NSString *key;                ///< key

/**
 消息data数据
 */
@property (nonatomic, strong) NSData *value;                ///< value

/**
 消息文件名
 */
@property (nullable, nonatomic, strong) NSString *filename; ///< filename (nil if inline)

/**
 消息大小（字节）
 */
@property (nonatomic) int size;                             ///< value's size in bytes

/**
 消息修改时间
 */
@property (nonatomic) int modTime;                          ///< modification unix timestamp

/**
 消息导入时间
 */
@property (nonatomic) int accessTime;                       ///< last access unix timestamp

/**
 拓展数据
 */
@property (nullable, nonatomic, strong) NSData *extendedData; ///< extended data (nil if no extended data)
@end

/**
 Storage type, indicated where the `YYKVStorageItem.value` stored.
 
 @discussion Typically, write data to sqlite is faster than extern file, but 
 reading performance is dependent on data size. In my test (on iPhone 6 64G), 
 read data from extern file is faster than from sqlite when the data is larger 
 than 20KB.
 
 * If you want to store large number of small datas (such as contacts cache), 
   use YYKVStorageTypeSQLite to get better performance.
 * If you want to store large files (such as image cache),
   use YYKVStorageTypeFile to get better performance.
 * You can use YYKVStorageTypeMixed and choice your storage type for each item.
 
 See <http://www.sqlite.org/intern-v-extern-blob.html> for more information.
 
 消息存储类型，表示存储“YYKVStorageItem.value”的位置。
 
 一般而言,数据存入sqlite比写文件更快，但读取数据的性能依赖数据大小 以iPhone 6 64G为例
 数据超过20KB，则从文件读取数据比sqlite读取更快
 存储较小的数据使用 YYKVStorageTypeSQLite 获得更好的性能
 如果存储较大的数据 如图片数据, 使用 YYKVStorageTypeFile 获取更好的性能
 使用 LSMessageDiskStorageTypeMixed 将针对每一个item采用不同的存储方式
 详见 http://www.sqlite.org/intern-v-extern-blob.html
 
 */
typedef NS_ENUM(NSUInteger, YYKVStorageType) {
    
    /// The `value` is stored as a file in file system.
    /// 消息存入文件
    YYKVStorageTypeFile = 0,
    
    /// The `value` is stored in sqlite with blob type.
    /// 消息存入sqlite，采用blob的类型
    YYKVStorageTypeSQLite = 1,
    
    /// The `value` is stored in file system or sqlite based on your choice.
    /// 根据选择选取存入方式
    YYKVStorageTypeMixed = 2,
};



/**
 YYKVStorage is a key-value storage based on sqlite and file system.
 Typically, you should not use this class directly.
 
 @discussion The designated initializer for YYKVStorage is `initWithPath:type:`. 
 After initialized, a directory is created based on the `path` to hold key-value data.
 Once initialized you should not read or write this directory without the instance.
 
 You may compile the latest version of sqlite and ignore the libsqlite3.dylib in
 iOS system to get 2x~4x speed up.
 
 @warning The instance of this class is *NOT* thread safe, you need to make sure 
 that there's only one thread to access the instance at the same time. If you really 
 need to process large amounts of data in multi-thread, you should split the data
 to multiple KVStorage instance (sharding).
 
 消息写入file/sqlite的管理类
 @discussion 键值对的方式将消息存入文件和sqlite 使用`initWithPath:type:`进行初始化
 初始化后 不要再对生成的path进行读写操作 使用最新的sqlite版本获取2-4倍的速度提升
 产生的实例并不是线程安全的，应该在同一时间在同一的线程使用，数据较大时，应该对数据进行拆分成多个片段进行存储
 */
@interface YYKVStorage : NSObject

#pragma mark - Attribute
///=============================================================================
/// @name Attribute
///=============================================================================

/**
 消息存入的路径
 */
@property (nonatomic, readonly) NSString *path;        ///< The path of this storage.

/**
 消息存储类型
 */
@property (nonatomic, readonly) YYKVStorageType type;  ///< The type of this storage.

/**
 是否打印log
 */
@property (nonatomic) BOOL errorLogsEnabled;           ///< Set `YES` to enable error logs for debug.

#pragma mark - 初始化
///=============================================================================
/// @name Initializer
///=============================================================================
- (instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (instancetype)new UNAVAILABLE_ATTRIBUTE;

/**
 The designated initializer. 
 
 @param path  Full path of a directory in which the storage will write data. If
    the directory is not exists, it will try to create one, otherwise it will 
    read the data in this directory.
 @param type  The storage type. After first initialized you should not change the 
    type of the specified path.
 @return  A new storage object, or nil if an error occurs.
 @warning Multiple instances with the same path will make the storage unstable.
 
 推荐的实例化方法
 path  写数据的路径. 如果路径存在，则会在此路径读写数据 否则建立一个新路径
 type  存储类型  一旦设置后不要修改
 返回一个存储管理实例, 发生错误返回nil
 多个实例操作同一个路径 会导致错误
 */
- (nullable instancetype)initWithPath:(NSString *)path type:(YYKVStorageType)type NS_DESIGNATED_INITIALIZER;


#pragma mark - 保存消息
///=============================================================================
/// @name Save Items
///=============================================================================

/**
 Save an item or update the item with 'key' if it already exists.
 
 @discussion This method will save the item.key, item.value, item.filename and
 item.extendedData to disk or sqlite, other properties will be ignored. item.key 
 and item.value should not be empty (nil or zero length).
 
 If the `type` is YYKVStorageTypeFile, then the item.filename should not be empty.
 If the `type` is YYKVStorageTypeSQLite, then the item.filename will be ignored.
 It the `type` is YYKVStorageTypeMixed, then the item.value will be saved to file 
 system if the item.filename is not empty, otherwise it will be saved to sqlite.
 
 @param item  An item.
 @return Whether succeed.
 
 保存item key值存在时更新item
 @discussion 此方法会将 item.key, item.value, item.filename 和
 item.extendedData 写入文件或sqlite, 其他属性会忽略. item.key
 和 item.value 不应该为空 (nil || length == 0).
 item  消息item
 返回是否成功
 */
- (BOOL)saveItem:(YYKVStorageItem *)item;

/**
 Save an item or update the item with 'key' if it already exists.
 
 @discussion This method will save the key-value pair to sqlite. If the `type` is
 YYKVStorageTypeFile, then this method will failed.
 
 @param key   The key, should not be empty (nil or zero length).
 @param value The key, should not be empty (nil or zero length).
 @return Whether succeed.
 
 保存item key值存在时更新item
 此方法会保存键值对到 sqlite. 如果存储类型为 YYKVStorageTypeFile , 此方法会失败
 key   key值不能为空
 value value不能为空
 返回是否成功
 */
- (BOOL)saveItemWithKey:(NSString *)key value:(NSData *)value;

/**
 Save an item or update the item with 'key' if it already exists.
 
 @discussion
 If the `type` is YYKVStorageTypeFile, then the `filename` should not be empty.
 If the `type` is YYKVStorageTypeSQLite, then the `filename` will be ignored.
 It the `type` is YYKVStorageTypeMixed, then the `value` will be saved to file
 system if the `filename` is not empty, otherwise it will be saved to sqlite.
 
 @param key           The key, should not be empty (nil or zero length).
 @param value         The key, should not be empty (nil or zero length).
 @param filename      The filename.
 @param extendedData  The extended data for this item (pass nil to ignore it).
 
 @return Whether succeed.
 
 
 保存item key值存在时更新item
 如果写入类型为LSMessageDiskStorageTypeFile,filename 不能为空
 如果写入类型为LSMessageDiskStorageTypeSQLite, filename 会被忽略
 如果写入类型为LSMessageDiskStorageTypeMixed, 如果filename不为空 则value会被存入文件 否则存入sqlite
 key           key值不能为空
 value         value不能为空
 filename      文件名
 extendedData  item的拓展数据 如果是nil则忽略
 Whether succeed.
 */
- (BOOL)saveItemWithKey:(NSString *)key
                  value:(NSData *)value
               filename:(nullable NSString *)filename
           extendedData:(nullable NSData *)extendedData;

#pragma mark - 删除消息
///=============================================================================
/// @name Remove Items
///=============================================================================

/**
 Remove an item with 'key'.
 
 @param key The item's key.
 @return Whether succeed.
 
 根据key值删除item
 keys 特定的key值
 返回是否删除成功
 */
- (BOOL)removeItemForKey:(NSString *)key;

/**
 Remove items with an array of keys.
 
 @param keys An array of specified keys.
 
 @return Whether succeed.
 
 根据keys数组删除items
 keys keys数组
 返回是否删除成功
 */
- (BOOL)removeItemForKeys:(NSArray<NSString *> *)keys;

/**
 Remove all items which `value` is larger than a specified size.
 
 @param size  The maximum size in bytes.
 @return Whether succeed.
 
 根据消息value的开销限制删除items
 size 消息value的最大限制
 返回是否删除成功
 */
- (BOOL)removeItemsLargerThanSize:(int)size;

/**
 Remove all items which last access time is earlier than a specified timestamp.
 
 @param time  The specified unix timestamp.
 @return Whether succeed.
 
 删除比指定时间更早存入的消息
 time  指定的时间
 返回是否删除成功
 */
- (BOOL)removeItemsEarlierThanTime:(int)time;

/**
 Remove items to make the total size not larger than a specified size.
 The least recently used (LRU) items will be removed first.
 
 @param maxSize The specified size in bytes.
 @return Whether succeed.
 
 根据消息开销限制删除items (LRU对象优先删除)
 maxCount 最大的消息开销
 返回是否删除成功
 */
- (BOOL)removeItemsToFitSize:(int)maxSize;

/**
 Remove items to make the total count not larger than a specified count.
 The least recently used (LRU) items will be removed first.
 
 @param maxCount The specified item count.
 @return Whether succeed.
 
 根据消息数量限制删除items (LRU对象优先删除)
 maxCount 最大的消息数量
 返回是否删除成功
 */
- (BOOL)removeItemsToFitCount:(int)maxCount;

/**
 Remove all items in background queue.
 
 @discussion This method will remove the files and sqlite database to a trash
 folder, and then clear the folder in background queue. So this method is much 
 faster than `removeAllItemsWithProgressBlock:endBlock:`.
 
 @return Whether succeed.
 
 在后台队列中，删除所有的item
 @discussion 此方法会删除 files 和 sqlite database 进入回收站 并在后台清除回收站数据
 比`removeAllItemsWithProgressBlock:endBlock:`方法更快
 @return 返回是否删除成功
 */
- (BOOL)removeAllItems;

/**
 Remove all items.
 
 @warning You should not send message to this instance in these blocks.
 @param progress This block will be invoked during removing, pass nil to ignore.
 @param end      This block will be invoked at the end, pass nil to ignore.
 
 删除所有的item
 @warning 在block中不要对该实例发送消息
 progress 删除时执行的block，nil则不执行
 end      删除结束执行的block，nil则不执行
 */
- (void)removeAllItemsWithProgressBlock:(nullable void(^)(int removedCount, int totalCount))progress
                               endBlock:(nullable void(^)(BOOL error))end;


#pragma mark - 获取消息
///=============================================================================
/// @name Get Items
///=============================================================================

/**
 Get item with a specified key.
 
 @param key A specified key.
 @return Item for the key, or nil if not exists / error occurs.
 
 根据key获取item
 key  特定的key值
 返回item, 发送错误返回nil
 */
- (nullable YYKVStorageItem *)getItemForKey:(NSString *)key;

/**
 Get item information with a specified key.
 The `value` in this item will be ignored.
 
 @param key A specified key.
 @return Item information for the key, or nil if not exists / error occurs.
 
 根据key获取item的信息（value会被忽略）
 key  特定的key值
 返回item的信息, 发送错误返回nil
 */
- (nullable YYKVStorageItem *)getItemInfoForKey:(NSString *)key;

/**
 Get item value with a specified key.
 
 @param key  A specified key.
 @return Item's value, or nil if not exists / error occurs.
 
 根据key获取item的value
 key  特定的key值
 返回item的value, 发送错误返回nil
 */
- (nullable NSData *)getItemValueForKey:(NSString *)key;

/**
 Get items with an array of keys.
 
 @param keys  An array of specified keys.
 @return An array of `YYKVStorageItem`, or nil if not exists / error occurs.
 
 根据key的数组获取item的信息
 keys  key值的数组
 包含`YYKVStorageItem`的数组 发生错误返回nil
 */
- (nullable NSArray<YYKVStorageItem *> *)getItemForKeys:(NSArray<NSString *> *)keys;

/**
 Get item infomartions with an array of keys.
 The `value` in items will be ignored.
 
 @param keys  An array of specified keys.
 @return An array of `YYKVStorageItem`, or nil if not exists / error occurs.
 
 根据key的数组获取item的信息（value会被忽略）
 keys  key值的数组
 包含`LSMessageDiskStorageItem`的数组 发生错误返回nil
 */
- (nullable NSArray<YYKVStorageItem *> *)getItemInfoForKeys:(NSArray<NSString *> *)keys;

/**
 Get items value with an array of keys.
 
 @param keys  An array of specified keys.
 @return A dictionary which key is 'key' and value is 'value', or nil if not 
    exists / error occurs.
 
 根据一个key值数组获取item和key的字典
 keys  key值的数组
 返回一个字典 key->item对应 发生错误返回nil
 */
- (nullable NSDictionary<NSString *, NSData *> *)getItemValueForKeys:(NSArray<NSString *> *)keys;

#pragma mark - 获取存储属性
///=============================================================================
/// @name Get Storage Status
///=============================================================================

/**
 Whether an item exists for a specified key.
 
 @param key  A specified key.
 
 @return `YES` if there's an item exists for the key, `NO` if not exists or an error occurs.
 
 根据key值查找item是否存在
 key  特定的key
 返回item是否存在
 */
- (BOOL)itemExistsForKey:(NSString *)key;

/**
 Get total item count.
 @return Total item count, -1 when an error occurs.
 
 获取item的总数
 返回总数，如果发生错误返回-1
 */
- (int)getItemsCount;

/**
 Get item value's total size in bytes.
 @return Total size in bytes, -1 when an error occurs.
 
 获取items的总大小（字节）
 返回总大小，如果发生错误返回-1
 */
- (int)getItemsSize;

@end

NS_ASSUME_NONNULL_END
