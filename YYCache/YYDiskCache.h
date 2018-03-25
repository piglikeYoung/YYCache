//
//  YYDiskCache.h
//  YYCache <https://github.com/ibireme/YYCache>
//
//  Created by ibireme on 15/2/11.
//  Copyright (c) 2015 ibireme.
//
//  This source code is licensed under the MIT-style license found in the
//  LICENSE file in the root directory of this source tree.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 YYDiskCache is a thread-safe cache that stores key-value pairs backed by SQLite
 and file system (similar to NSURLCache's disk cache).
 
 YYDiskCache has these features:
 
 * It use LRU (least-recently-used) to remove objects.
 * It can be controlled by cost, count, and age.
 * It can be configured to automatically evict objects when there's no free disk space.
 * It can automatically decide the storage type (sqlite/file) for each object to get
      better performance.
 
 You may compile the latest version of sqlite and ignore the libsqlite3.dylib in
 iOS system to get 2x~4x speed up.
 */
@interface YYDiskCache : NSObject

#pragma mark - Attribute
///=============================================================================
/// @name Attribute
///=============================================================================

/**
 The name of the cache. Default is nil.
 磁盘cache的名称
 */
@property (nullable, copy) NSString *name;

/**
 The path of the cache (read-only).
 磁盘cache的文件路径
 */
@property (readonly) NSString *path;

/**
 If the object's data size (in bytes) is larger than this value, then object will
 be stored as a file, otherwise the object will be stored in sqlite.
 
 0 means all objects will be stored as separated files, NSUIntegerMax means all
 objects will be stored in sqlite. 
 
 The default value is 20480 (20KB).
 
 如果存入的消息超过此值，则消息会存入文件file，否则存入sqlite
 0 意味着所有的消息会存入不同的文件file, NSUIntegerMax 意味着所有的消息会存入 sqlite.
 默认的值为 20480 (20KB).
 */
@property (readonly) NSUInteger inlineThreshold;

/**
 If this block is not nil, then the block will be used to archive object instead
 of NSKeyedArchiver. You can use this block to support the objects which do not
 conform to the `NSCoding` protocol.
 
 The default value is nil.
 
 如果block为nil 则会使用NSKeyedArchiver归档消息 使用此block以支持未遵循`NSCoding` 协议的对象存储
 默认值为nil
 */
@property (nullable, copy) NSData *(^customArchiveBlock)(id object);

/**
 If this block is not nil, then the block will be used to unarchive object instead
 of NSKeyedUnarchiver. You can use this block to support the objects which do not
 conform to the `NSCoding` protocol.
 
 The default value is nil.
 
 block不为nil则使用自定义的解归档方法替代 NSKeyedUnarchiver. 使用此block以支持未遵循`NSCoding` 协议的对象
 默认值为nil
 */
@property (nullable, copy) id (^customUnarchiveBlock)(NSData *data);

/**
 When an object needs to be saved as a file, this block will be invoked to generate
 a file name for a specified key. If the block is nil, the cache use md5(key) as 
 default file name.
 
 The default value is nil.
 
 当需要写文件时, block 会生成文件名和一个key，如果block是nil 则cache使用MD5生成默认的文件名
 默认值为nil
 */
@property (nullable, copy) NSString *(^customFileNameBlock)(NSString *key);



#pragma mark - Limit
///=============================================================================
/// @name Limit
///=============================================================================

/**
 The maximum number of objects the cache should hold.
 
 @discussion The default value is NSUIntegerMax, which means no limit.
 This is not a strict limit — if the cache goes over the limit, some objects in the
 cache could be evicted later in background queue.
 
 消息池子cache中存储的最大数量
 默认的值为 NSUIntegerMax 表示无限制
 它并不是一个严格的限制 - 如果缓存超过限制，那么一些缓存对象就会在后台队列中被回收。
 */
@property NSUInteger countLimit;

/**
 The maximum total cost that the cache can hold before it starts evicting objects.
 
 @discussion The default value is NSUIntegerMax, which means no limit.
 This is not a strict limit — if the cache goes over the limit, some objects in the
 cache could be evicted later in background queue.
 
 消息池子cache中容许的最大开销
 默认的值为 NSUIntegerMax 表示无限制
 它并不是一个严格的限制 - 如果缓存超过限制，那么一些缓存对象就会在后台队列中被回收。
 */
@property NSUInteger costLimit;

/**
 The maximum expiry time of objects in cache.
 
 @discussion The default value is DBL_MAX, which means no limit.
 This is not a strict limit — if an object goes over the limit, the objects could
 be evicted later in background queue.
 
 消息池子cache中容许的时间限制
 默认的值为 DBL_MAX 表示无限制
 它并不是一个严格的限制 - 如果缓存超过限制，那么一些缓存对象就会在后台队列中被回收。
 */
@property NSTimeInterval ageLimit;

/**
 The minimum free disk space (in bytes) which the cache should kept.
 
 @discussion The default value is 0, which means no limit.
 If the free disk space is lower than this value, the cache will remove objects
 to free some disk space. This is not a strict limit—if the free disk space goes
 over the limit, the objects could be evicted later in background queue.
 
 cache保证的最小磁盘disk空闲
 默认值为 0, 意味着无限制
 如果disk空闲容量小于此值，将移除对象释放内存
 */
@property NSUInteger freeDiskSpaceLimit;

/**
 The auto trim check time interval in seconds. Default is 60 (1 minute).
 
 @discussion The cache holds an internal timer to check whether the cache reaches
 its limits, and if the limit is reached, it begins to evict objects.
 
 自动检测容器限制 默认时间60.0s
 cache消息池子持有Timer,以确保cache是否达到上限 如果达到上限则进行削减
 */
@property NSTimeInterval autoTrimInterval;

/**
 Set `YES` to enable error logs for debug.
 
 设置`YES` 容许错误log
 */
@property BOOL errorLogsEnabled;

#pragma mark - Initializer
///=============================================================================
/// @name Initializer
///=============================================================================
- (instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (instancetype)new UNAVAILABLE_ATTRIBUTE;

/**
 Create a new cache based on the specified path.
 
 @param path Full path of a directory in which the cache will write data.
     Once initialized you should not read and write to this directory.
 
 @return A new cache object, or nil if an error occurs.
 
 @warning If the cache instance for the specified path already exists in memory,
     this method will return it directly, instead of creating a new instance.
 
 根据path实例化磁盘cache对象
 path cache写入消息的全路径 实例化后，不要在此路径读写数据
 返回 cache 对象, 如果发生错误返回nil
 如果path已经存在内存中，则会直接返回cache对象 取代创建对象
 */
- (nullable instancetype)initWithPath:(NSString *)path;

/**
 The designated initializer.
 
 @param path       Full path of a directory in which the cache will write data.
     Once initialized you should not read and write to this directory.
 
 @param threshold  The data store inline threshold in bytes. If the object's data
     size (in bytes) is larger than this value, then object will be stored as a 
     file, otherwise the object will be stored in sqlite. 0 means all objects will 
     be stored as separated files, NSUIntegerMax means all objects will be stored 
     in sqlite. If you don't know your object's size, 20480 is a good choice.
     After first initialized you should not change this value of the specified path.
 
 @return A new cache object, or nil if an error occurs.
 
 @warning If the cache instance for the specified path already exists in memory,
     this method will return it directly, instead of creating a new instance.
 
 推荐的实例化方法
 path cache写入消息的全路径 实例化后，不要在此路径读写数据
 threshold  存入数据尺寸的限制. 如果存入sqlite数据字节数超过此值 则会写入文件,
 0 意味着所有的消息会存入不同的文件file, NSUIntegerMax 意味着所有的消息会存入 sqlite 推荐值为20480
 返回 cache 对象, 如果发生错误返回nil
 如果path已经存在内存中，则会直接返回cache对象 取代创建对象
 */
- (nullable instancetype)initWithPath:(NSString *)path
                      inlineThreshold:(NSUInteger)threshold NS_DESIGNATED_INITIALIZER;


#pragma mark - Access Methods
///=============================================================================
/// @name Access Methods
///=============================================================================

/**
 Returns a boolean value that indicates whether a given key is in cache.
 This method may blocks the calling thread until file read finished.
 
 @param key A string identifying the value. If nil, just return NO.
 @return Whether the key is in cache.
 
 返回一个boolean 表示给定的key是否存在disk的cache中 此方法会堵塞直到返回
 key 标识消息对象的key 如果为nil 则返回NO
 返回key是否存在cache中
 */
- (BOOL)containsObjectForKey:(NSString *)key;

/**
 Returns a boolean value with the block that indicates whether a given key is in cache.
 This method returns immediately and invoke the passed block in background queue 
 when the operation finished.
 
 @param key   A string identifying the value. If nil, just return NO.
 @param block A block which will be invoked in background queue when finished.
 
 返回一个boolean 表示给定的key是否存在disk的cache中 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 key   标识消息对象的key 如果为nil 则返回NO
 block 在后台线程执行完成后的回调block
 */
- (void)containsObjectForKey:(NSString *)key withBlock:(void(^)(NSString *key, BOOL contains))block;

/**
 Returns the value associated with a given key.
 This method may blocks the calling thread until file read finished.
 
 @param key A string identifying the value. If nil, just return nil.
 @return The value associated with key, or nil if no value is associated with key.
 
 返回指定key对应的消息 此方法会堵塞直到返回
 key 标识消息对象的key 如果为nil 则返回nil
 返回key对应的, 如果未找到，则返回nil
 */
- (nullable id<NSCoding>)objectForKey:(NSString *)key;

/**
 Returns the value associated with a given key.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param key A string identifying the value. If nil, just return nil.
 @param block A block which will be invoked in background queue when finished.
 
 返回指定key对应的消息  此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 key 标识消息对象的key 如果为nil 则返回nil
 block 在后台线程执行完成后的回调block
 */
- (void)objectForKey:(NSString *)key withBlock:(void(^)(NSString *key, id<NSCoding> _Nullable object))block;

/**
 Sets the value of the specified key in the cache.
 This method may blocks the calling thread until file write finished.
 
 @param object The object to be stored in the cache. If nil, it calls `removeObjectForKey:`.
 @param key    The key with which to associate the value. If nil, this method has no effect.
 
 将消息和对应的key值存入cache中 此方法会堵塞直到写入数据完成
 object 存入cache中的消息对象. 如果是nil则会调用`removeObjectForKey:`.
 key    和消息对象关联的key. 如果为nil则不会操作
 */
- (void)setObject:(nullable id<NSCoding>)object forKey:(NSString *)key;

/**
 Sets the value of the specified key in the cache.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param object The object to be stored in the cache. If nil, it calls `removeObjectForKey:`.
 @param block  A block which will be invoked in background queue when finished.
 
 将消息和对应的key值存入cache中 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 object 存入cache中的消息对象. 如果是nil则会调用`removeObjectForKey:`.
 key    和消息对象关联的key. 如果为nil则不会操作
 block  在后台执行完后的回调block
 */
- (void)setObject:(nullable id<NSCoding>)object forKey:(NSString *)key withBlock:(void(^)(void))block;

/**
 Removes the value of the specified key in the cache.
 This method may blocks the calling thread until file delete finished.
 
 @param key The key identifying the value to be removed. If nil, this method has no effect.
 
 删除cache中指定key对应的消息 此方法会堵塞直到文件删除完成
 key 标识删除对象的key 如果为nil则不会操作
 */
- (void)removeObjectForKey:(NSString *)key;

/**
 Removes the value of the specified key in the cache.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param key The key identifying the value to be removed. If nil, this method has no effect.
 @param block  A block which will be invoked in background queue when finished.
 
 删除cache中指定key对应的消息 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 key 标识删除对象的key 如果为nil则不会操作
 block  在后台执行完后的回调block
 */
- (void)removeObjectForKey:(NSString *)key withBlock:(void(^)(NSString *key))block;

/**
 Empties the cache.
 This method may blocks the calling thread until file delete finished.
 
 删除cache中所有的对象 此方法会堵塞直到cache清除完成
 */
- (void)removeAllObjects;

/**
 Empties the cache.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param block  A block which will be invoked in background queue when finished.
 
 删除cache中所有的对象 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 block  在后台执行完后的回调block
 */
- (void)removeAllObjectsWithBlock:(void(^)(void))block;

/**
 Empties the cache with block.
 This method returns immediately and executes the clear operation with block in background.
 
 @warning You should not send message to this instance in these blocks.
 @param progress This block will be invoked during removing, pass nil to ignore.
 @param end      This block will be invoked at the end, pass nil to ignore.
 
 删除cache中所有的对象 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 不要在block中对该对象发送消息
 progress 删除过程中执行, nil的话忽略
 end      删除完成后执行, nil的话忽略
 */
- (void)removeAllObjectsWithProgressBlock:(nullable void(^)(int removedCount, int totalCount))progress
                                 endBlock:(nullable void(^)(BOOL error))end;


/**
 Returns the number of objects in this cache.
 This method may blocks the calling thread until file read finished.
 
 @return The total objects count.
 
 返回cache中的消息总数量 此方法会堵塞直到读取完成
 返回消息总数
 */
- (NSInteger)totalCount;

/**
 Get the number of objects in this cache.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param block  A block which will be invoked in background queue when finished.
 
 获取cache中的消息总数量 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 block  在后台执行完后的回调block
 */
- (void)totalCountWithBlock:(void(^)(NSInteger totalCount))block;

/**
 Returns the total cost (in bytes) of objects in this cache.
 This method may blocks the calling thread until file read finished.
 
 @return The total objects cost in bytes.
 
 返回cache中的消息总开销（字节） 此方法会堵塞直到读取完成
 返回消息总开销（字节）
 */
- (NSInteger)totalCost;

/**
 Get the total cost (in bytes) of objects in this cache.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param block  A block which will be invoked in background queue when finished.
 
 返回cache中的消息总开销（字节）此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 block  在后台执行完后的回调block
 */
- (void)totalCostWithBlock:(void(^)(NSInteger totalCost))block;


#pragma mark - Trim
///=============================================================================
/// @name Trim
///=============================================================================

/**
 Removes objects from the cache use LRU, until the `totalCount` is below the specified value.
 This method may blocks the calling thread until operation finished.
 
 @param count  The total count allowed to remain after the cache has been trimmed.
 
 一旦 `totalCount` 高于总数限制，则删除消息 将LRU对象放入缓存区 此方法会堵塞直到完成
 count  清除消息后容许的消息总数量
 */
- (void)trimToCount:(NSUInteger)count;

/**
 Removes objects from the cache use LRU, until the `totalCount` is below the specified value.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param count  The total count allowed to remain after the cache has been trimmed.
 @param block  A block which will be invoked in background queue when finished.
 
 一旦 `totalCount` 高于总数限制，则删除消息 将LRU对象放入缓存区 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 count  清除消息后容许的消息总数量
 block 完成后的回调
 */
- (void)trimToCount:(NSUInteger)count withBlock:(void(^)(void))block;

/**
 Removes objects from the cache use LRU, until the `totalCost` is below the specified value.
 This method may blocks the calling thread until operation finished.
 
 @param cost The total cost allowed to remain after the cache has been trimmed.
 
 一旦 `totalCount` 高于总开销限制，则删除消息 将LRU对象放入缓存区 此方法会堵塞直到完成
 count  清除消息后容许的消息总开销
 */
- (void)trimToCost:(NSUInteger)cost;

/**
 Removes objects from the cache use LRU, until the `totalCost` is below the specified value.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param cost The total cost allowed to remain after the cache has been trimmed.
 @param block  A block which will be invoked in background queue when finished.
 
 一旦 `totalCount` 高于总开销限制，则删除消息 将LRU对象放入缓存区 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 count  清除消息后容许的消息总开销
 block 完成后的回调
 */
- (void)trimToCost:(NSUInteger)cost withBlock:(void(^)(void))block;

/**
 Removes objects from the cache use LRU, until all expiry objects removed by the specified value.
 This method may blocks the calling thread until operation finished.
 
 @param age  The maximum age of the object.
 
 按照时间限制削减 （LRU对象进入缓冲区）此方法会堵塞
 age  最大的时间 seconds.
 */
- (void)trimToAge:(NSTimeInterval)age;

/**
 Removes objects from the cache use LRU, until all expiry objects removed by the specified value.
 This method returns immediately and invoke the passed block in background queue
 when the operation finished.
 
 @param age  The maximum age of the object.
 @param block  A block which will be invoked in background queue when finished.
 
 一旦 按照时间限制削减 将LRU对象放入缓存区 此方法会立即返回，并在后台线程中执行，直到执行完成调用block回调
 age  最大的时间 seconds.
 block 完成后的回调
 */
- (void)trimToAge:(NSTimeInterval)age withBlock:(void(^)(void))block;


#pragma mark - Extended Data
///=============================================================================
/// @name Extended Data
///=============================================================================

/**
 Get extended data from an object.
 
 @discussion See 'setExtendedData:toObject:' for more information.
 
 @param object An object.
 @return The extended data.
 
 获取消息的拓展数据
 详见'setExtendedData:toObject:'
 object 消息对象
 拓展数据
 */
+ (nullable NSData *)getExtendedDataFromObject:(id)object;

/**
 Set extended data to an object.
 
 @discussion You can set any extended data to an object before you save the object
 to disk cache. The extended data will also be saved with this object. You can get
 the extended data later with "getExtendedDataFromObject:".
 
 @param extendedData The extended data (pass nil to remove).
 @param object       The object.
 
 设置消息的拓展数据
 当保存消息到cache之前可以设置消息的拓展数据 拓展数据会同样存入cache中 你可以使用"getExtendedDataFromObject:"获取拓展数据
 extendedData 拓展数据 (如果是nil 则删除数据)
 object       对应的消息
 */
+ (void)setExtendedData:(nullable NSData *)extendedData toObject:(id)object;

@end

NS_ASSUME_NONNULL_END
