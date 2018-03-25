//
//  YYMemoryCache.h
//  YYCache <https://github.com/ibireme/YYCache>
//
//  Created by ibireme on 15/2/7.
//  Copyright (c) 2015 ibireme.
//
//  This source code is licensed under the MIT-style license found in the
//  LICENSE file in the root directory of this source tree.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 YYMemoryCache is a fast in-memory cache that stores key-value pairs.
 In contrast to NSDictionary, keys are retained and not copied.
 The API and performance is similar to `NSCache`, all methods are thread-safe.
 
 YYMemoryCache objects differ from NSCache in a few ways:
 
 * It uses LRU (least-recently-used) to remove objects; NSCache's eviction method
   is non-deterministic.
 * It can be controlled by cost, count and age; NSCache's limits are imprecise.
 * It can be configured to automatically evict objects when receive memory 
   warning or app enter background.
 
 The time of `Access Methods` in YYMemoryCache is typically in constant time (O(1)).
 */
@interface YYMemoryCache : NSObject

#pragma mark - Attribute
///=============================================================================
/// @name Attribute
///=============================================================================

/**
    The name of the cache. Default is nil.
    cache 的名称，默认为nil
 */
@property (nullable, copy) NSString *name;

/**
    The number of objects in the cache (read-only)
    memory 中的消息总数
 */
@property (readonly) NSUInteger totalCount;

/**
    The total cost of objects in the cache (read-only).
    memory 中的消息总开销
 */
@property (readonly) NSUInteger totalCost;


#pragma mark - Limit
///=============================================================================
/// @name Limit
///=============================================================================

/**
 The maximum number of objects the cache should hold.
 
 @discussion The default value is NSUIntegerMax, which means no limit.
 This is not a strict limit—if the cache goes over the limit, some objects in the
 cache could be evicted later in backgound thread.
 
 消息池子 cache 中存储的最大数量
 默认的值为 NSUIntegerMax 表示无限制
 如果超过此限制，则稍后在后台线程将清除一些对象
 */
@property NSUInteger countLimit;

/**
 The maximum total cost that the cache can hold before it starts evicting objects.
 
 @discussion The default value is NSUIntegerMax, which means no limit.
 This is not a strict limit—if the cache goes over the limit, some objects in the
 cache could be evicted later in backgound thread.
 
 消息池子cache中容许的最大开销
 默认的值为 NSUIntegerMax 表示无限制
 如果超过此限制，则稍后在后台线程将清除一些对象
 */
@property NSUInteger costLimit;

/**
 The maximum expiry time of objects in cache.
 
 @discussion The default value is DBL_MAX, which means no limit.
 This is not a strict limit—if an object goes over the limit, the object could 
 be evicted later in backgound thread.
 
 消息池子cache中容许的时间限制
 默认的值为 DBL_MAX 表示无限制
 如果超过此限制，则稍后在后台线程将清除一些对象
 */
@property NSTimeInterval ageLimit;

/**
 The auto trim check time interval in seconds. Default is 5.0.
 
 @discussion The cache holds an internal timer to check whether the cache reaches 
 its limits, and if the limit is reached, it begins to evict objects.
 
 自动检测容器限制 默认时间 5.0s
 cache 消息池子持有 Timer，以确保 cache 是否达到上限 如果达到上限则进行削减
 */
@property NSTimeInterval autoTrimInterval;

/**
 If `YES`, the cache will remove all objects when the app receives a memory warning.
 The default value is `YES`.
 
 如果是 YES 则收到内存报警时会删除所有的 cache 消息对象
 默认值是 YES
 */
@property BOOL shouldRemoveAllObjectsOnMemoryWarning;

/**
 If `YES`, The cache will remove all objects when the app enter background.
 The default value is `YES`.
 
 如果是 YES 则收到app进入后台时会删除所有的cache消息对象
 默认值是 YES
 */
@property BOOL shouldRemoveAllObjectsWhenEnteringBackground;

/**
 A block to be executed when the app receives a memory warning.
 The default value is nil.
 
 app 收到报警时执行的 block 默认为 nil
 */
@property (nullable, copy) void(^didReceiveMemoryWarningBlock)(YYMemoryCache *cache);

/**
 A block to be executed when the app enter background.
 The default value is nil.
 
 app 收到进入后台时执行的 block 默认为 nil
 */
@property (nullable, copy) void(^didEnterBackgroundBlock)(YYMemoryCache *cache);

/**
 If `YES`, the key-value pair will be released on main thread, otherwise on
 background thread. Default is NO.
 
 @discussion You may set this value to `YES` if the key-value object contains
 the instance which should be released in main thread (such as UIView/CALayer).
 
 键值对是否在主线程删除 默认值为NO.
 仅仅当键值对中包含 UIView、CALayer 等非线程安全对象时，将值设为YES
 */
@property BOOL releaseOnMainThread;

/**
 If `YES`, the key-value pair will be released asynchronously to avoid blocking 
 the access methods, otherwise it will be released in the access method  
 (such as removeObjectForKey:). Default is YES.
 
 键值对异步的释放 默认值为 YES
 避免堵塞访问方法 否则将在 removeObjectForKey: 等方法中释放 默认是 YES
 */
@property BOOL releaseAsynchronously;


#pragma mark - Access Methods
///=============================================================================
/// @name Access Methods
///=============================================================================

/**
 Returns a Boolean value that indicates whether a given key is in cache.
 
 @param key An object identifying the value. If nil, just return `NO`.
 @return Whether the key is in cache.
 
 判断消息池子是否包含指定key的消息
 key 消息对象关联的key. 如果是nil则返回NO
 是否包含指定key的消息
 */
- (BOOL)containsObjectForKey:(id)key;

/**
 Returns the value associated with a given key.
 
 @param key An object identifying the value. If nil, just return nil.
 @return The value associated with key, or nil if no value is associated with key.
 
 获取与key关联的消息对象
 key 关联消息对象的 key,如果是 nil 则返回 nil
 返回与 key 关联的消息对象, 如果未找到则返回 nil
 */
- (nullable id)objectForKey:(id)key;

/**
 Sets the value of the specified key in the cache (0 cost).
 
 @param object The object to be stored in the cache. If nil, it calls `removeObjectForKey:`.
 @param key    The key with which to associate the value. If nil, this method has no effect.
 @discussion Unlike an NSMutableDictionary object, a cache does not copy the key 
 objects that are put into it.
 
 根据指定的 key 存储消息对象
 message 需要存储到池子的对象. 如果是nil则调用 `removeMessageForKey`.
 key 存储对象关联的key. 如果是nil则不执行任何操作
 与NSMutableDictionary相比, cache池子不会拷贝容器中的键值对
 */
- (void)setObject:(nullable id)object forKey:(id)key;

/**
 Sets the value of the specified key in the cache, and associates the key-value 
 pair with the specified cost.
 
 @param object The object to store in the cache. If nil, it calls `removeObjectForKey`.
 @param key    The key with which to associate the value. If nil, this method has no effect.
 @param cost   The cost with which to associate the key-value pair.
 @discussion Unlike an NSMutableDictionary object, a cache does not copy the key
 objects that are put into it.
 
 根据指定的key和开销cost存储消息
 object 需要存储到池子的对象. 如果是nil则调用 `removeObjectForKey`.
 key 存储对象关联的key. 如果是nil则不执行任何操作
 cost 关联键值对的开销
 与NSMutableDictionary相比, cache池子不会拷贝容器中的键值对
 */
- (void)setObject:(nullable id)object forKey:(id)key withCost:(NSUInteger)cost;

/**
 Removes the value of the specified key in the cache.
 
 @param key The key identifying the value to be removed. If nil, this method has no effect.
 
 根据指定的key删除消息
 key 需要删除的object的key. 如果是nil则不执行任何操作
 */
- (void)removeObjectForKey:(id)key;

/**
 Empties the cache immediately.
 
 删除所有的消息
 */
- (void)removeAllObjects;


#pragma mark - Trim
///=============================================================================
/// @name Trim
///=============================================================================

/**
 Removes objects from the cache with LRU, until the `totalCount` is below or equal to
 the specified value.
 @param count  The total count allowed to remain after the cache has been trimmed.
 */
- (void)trimToCount:(NSUInteger)count;

/**
 Removes objects from the cache with LRU, until the `totalCost` is or equal to
 the specified value.
 @param cost The total cost allowed to remain after the cache has been trimmed.
 */
- (void)trimToCost:(NSUInteger)cost;

/**
 Removes objects from the cache with LRU, until all expiry objects removed by the
 specified value.
 @param age  The maximum age (in seconds) of objects.
 */
- (void)trimToAge:(NSTimeInterval)age;

@end

NS_ASSUME_NONNULL_END
