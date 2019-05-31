import web


db = web.database(dbn='sqlite',
        db='AuctionBase' #TODO: add your SQLite database filename
    )

######################BEGIN HELPER METHODS######################

# Enforce foreign key constraints
# WARNING: DO NOT REMOVE THIS!
def enforceForeignKey():
    db.query('PRAGMA foreign_keys = ON')

# initiates a transaction on the database
def transaction():
    return db.transaction()
# Sample usage (in auctionbase.py):
#
# t = sqlitedb.transaction()
# try:
#     sqlitedb.query('[FIRST QUERY STATEMENT]')
#     sqlitedb.query('[SECOND QUERY STATEMENT]')
# except Exception as e:
#     t.rollback()
#     print str(e)
# else:
#     t.commit()
#
# check out http://webpy.org/cookbook/transactions for examples

# returns the current time from your database
def getTime():
    # TODO: update the query string to match
    # the correct column and table name in your database
    query_string = 'select Time from CurrentTime'
    results = query(query_string)
    # alternatively: return results[0]['currenttime']
    return results[0].Time # TODO: update this as well to match the
                                  # column name

# returns a single item specified by the Item's ID in the database
# Note: if the `result' list is empty (i.e. there are no items for a
# a given ID), this will throw an Exception!
def getItemById(item_id):
    # TODO: rewrite this method to catch the Exception in case `result' is empty
    query_string = 'select * from Items where itemID = $itemsID'
    result = db.query(query_string, {'itemsID': item_id})
    return result[0]

def getBid(item_id):
    query_string = 'select Bids.Time,UserID,Amount from Bids,CurrentTime where itemID = $itemID AND Bids.Time<=CurrentTime.Time'
    result = db.query(query_string, {'itemID':item_id})
    return result

def getCategory(item_id):
    query_string = 'select Category from Categories where itemID = $itemID'
    result = query(query_string, {'itemID':item_id})
    return result

def insertBid(user,item,amount):

    if amount !='' and user != '' and item != '':
        query_string = 'select Time from CurrentTime'
        result = db.query(query_string)
        time= result[0].Time
        
        try:
            sql_string = 'insert into Bids(ItemID,UserID,Amount,Time) values ($i,$u,$a,$t)'
            query_string = db.query(sql_string,{'i':item,'u':user,'a':amount,'t':time})
        except Exception as e:
            print str(e)
    check = 'select * from Bids where Amount = $a and ItemID = $i'
    check_query = db.query(check, {'a':amount,'i':item})
    return check_query

def getStatus(item):
    query_string = 'select Items.* from Items,CurrentTime where CurrentTime.Time BETWEEN Started AND Ends AND ItemID = $id'
    result = query(query_string, {'id':item})
    print (result)
    if len(result) ==0:
        return 'Closed'
    else :
        return 'Open'

def winner(item):
    query_string = 'select distinct Bids.* from Bids,Items,CurrentTime where Bids.Amount = Currently AND Bids.ItemID = $id AND Bids.ItemID = Items.ItemID AND ( Ends<=CurrentTime.Time OR Amount >= Buy_Price)' 
    result = query(query_string, {'id':item})
    print (result)
    if len(result) == 0: 
        return 
    else : 
        return result    
    
# wrapper method around web.py's db.query method
# check out http://webpy.org/cookbook/query for more info
def query(query_string, vars = {}):
    return list(db.query(query_string, vars))

def selectTime(times):
    query_string = 'update CurrentTime set time=$time'
    result = db.query(query_string,{'time':times})
    return result


def search(item_id,user_id,maxP,minP,stat,cat,ides):
    query_string = 'select Items.* from Items,CurrentTime,Categories where '
    flag=0
    if  item_id != '':    
        query_string = query_string + 'Items.itemID=$iID'
        flag=1     
    if  user_id != '':
        if flag:
            query_string = query_string + ' AND '        
        query_string = query_string + 'seller_UserID=$uID'
        flag=1
    if  maxP != '' :
        if flag:
            query_string = query_string + ' AND ' 
        query_string = query_string + 'Currently <= $maxPrice'
        flag=1;   
    if minP != '':
        if flag:
            query_string = query_string + ' AND '
        query_string = query_string + 'Currently >= $minPrice'
        flag=1
    if cat != '':
        if flag:
            query_string = query_string + ' AND '
        query_string = query_string + 'categories.ItemID=Items.ItemID AND categories.Category like $category'
        flag=1
    if ides != '':
        if flag:
            query_string = query_string + ' AND '
        query_string = query_string + 'Items.Description like $desc'
        flag=1
    if stat == 'all':
        if flag == 0: 
            query_string = 'select * from Items' 
    if stat == 'open':
        if flag:
            query_string = query_string + ' AND '
        query_string = query_string + 'CurrentTime.Time BETWEEN Started AND Ends'
    if stat == 'close':
        if flag:
            query_string = query_string + ' AND '
        query_string = query_string + 'Ends <= Time'
    if stat == 'notStarted':
        if flag:
            query_string = query_string + ' AND '
        query_string = query_string + 'Started > Time'  
    query_string = query_string + ' group by Items.ItemID'
    result = None
    print (query_string)
    try: 
        result = db.query (query_string, {'iID':item_id,'uID':user_id,'maxPrice':maxP,'minPrice':minP,'category':'%'+cat+'%','desc':'%'+ides+'%'})     
    except Exception as e:
        print str(e)
    return result
    
    

#####################END HELPER METHODS#####################

#TODO: additional methods to interact with your database,
# e.g. to update the current time
