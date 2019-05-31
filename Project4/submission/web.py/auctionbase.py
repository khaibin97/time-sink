#!/usr/bin/env python

import sys; sys.path.insert(0, 'lib') # this line is necessary for the rest
import os                             # of the imports to work!

import web
import sqlitedb
from jinja2 import Environment, FileSystemLoader
from datetime import datetime

###########################################################################################
##########################DO NOT CHANGE ANYTHING ABOVE THIS LINE!##########################
###########################################################################################

######################BEGIN HELPER METHODS######################

# helper method to convert times from database (which will return a string)
# into datetime objects. This will allow you to compare times correctly (using
# ==, !=, <, >, etc.) instead of lexicographically as strings.

# Sample use:
# current_time = string_to_time(sqlitedb.getTime())

def string_to_time(date_str):
    return datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S')

# helper method to render a template in the templates/ directory
#
# `template_name': name of template file to render
#
# `**context': a dictionary of variable names mapped to values
# that is passed to Jinja2's templating engine
#
# See curr_time's `GET' method for sample usage
#
# WARNING: DO NOT CHANGE THIS METHOD
def render_template(template_name, **context):
    extensions = context.pop('extensions', [])
    globals = context.pop('globals', {})

    jinja_env = Environment(autoescape=True,
            loader=FileSystemLoader(os.path.join(os.path.dirname(__file__), 'templates')),
            extensions=extensions,
            )
    jinja_env.globals.update(globals)

    web.header('Content-Type','text/html; charset=utf-8', unique=True)

    return jinja_env.get_template(template_name).render(context)

#####################END HELPER METHODS#####################

urls = ('/currtime', 'curr_time',
        '/selecttime', 'select_time',
        '/add_bid','add_bid',
        '/search','search',
        '/timetable','timetable',
        '/details-(.*)','details'
        # TODO: add additional URLs here
        # first parameter => URL, second parameter => class name
        )

class details:
    
    def GET(self,id):
        auction = web.input(id = None)
        item = sqlitedb.getItemById(auction.id)
        cat = sqlitedb.getCategory(auction.id)
        bid = sqlitedb.getBid(auction.id)
        status = sqlitedb.getStatus(auction.id)
        winner = sqlitedb.winner(auction.id)
        print (winner)

        return render_template('details.html', items = item, category = cat, bids = bid,status=status, result=winner)
    def POST(self):
        return render_templatef('details.html')
    
        
class search:
    def GET(self):
        return render_template('search.html')
    def POST(self):
        post_params = web.input()
        iID = post_params['itemID']
        usrID = post_params['userID']
        minP = post_params['minPrice']
        maxP = post_params['maxPrice']
        status = post_params['status']
        category = post_params['category']
        ides = post_params['itemDesc']

        res=sqlitedb.search(iID,usrID,maxP,minP,status,category,ides)
        #for list in res:
        #   print (list)
        #dict(mydict=res)        
        
        return render_template('search.html',search_result=res)

class add_bid:
    def GET(self):
        return render_template('add_bid.html')
    def POST(self):
        post_params = web.input()
        iID= post_params['itemID']
        uID = post_params['userID']
        amount=post_params['price']
        

        res = sqlitedb.insertBid(uID,iID,amount)
        print (res)
        return render_template('add_bid.html',add_result=res)

class curr_time:
    # A simple GET request, to '/currtime'
    #
    # Notice that we pass in `current_time' to our `render_template' call
    # in order to have its value displayed on the web page
    def GET(self):
        current_time = sqlitedb.getTime()
        return render_template('curr_time.html', time = current_time)

class select_time:
    # Aanother GET request, this time to the URL '/selecttime'
    def GET(self):
        return render_template('select_time.html')

    # A POST request
    #
    # You can fetch the parameters passed to the URL
    # by calling `web.input()' for **both** POST requests
    # and GET requests
    def POST(self):
        post_params = web.input()
        MM = post_params['MM']
        dd = post_params['dd']
        yyyy = post_params['yyyy']
        HH = post_params['HH']
        mm = post_params['mm']
        ss = post_params['ss']
        enter_name = post_params['entername']

        selected_time = '%s-%s-%s %s:%s:%s' % (yyyy, MM, dd, HH, mm, ss)
        try:
            sqlitedb.selectTime(selected_time)
        # TODO: save the selected time as the current time in the database
        except Exception as e:
            print str(e)
        update_message = '(Hello, %s. Previously selected time was: %s.)' % (enter_name, selected_time)

        # Here, we assign `update_message' to `message', which means
        # we'll refer to it in our template as `message'
        return render_template('select_time.html', message = update_message)

###########################################################################################
##########################DO NOT CHANGE ANYTHING BELOW THIS LINE!##########################
###########################################################################################

if __name__ == '__main__':
    web.internalerror = web.debugerror
    app = web.application(urls, globals())
    app.add_processor(web.loadhook(sqlitedb.enforceForeignKey))
    app.run()
