import pystache

renderer = pystache.Renderer()
parsed = pystache.parse(u'{{#people}}Hello, {{name}}.  You are {{age}} years old.\n{{/people}}')

for i in range(1000000):
    renderer.render(parsed, { "people": [
        { "name": "derek", "age": 20 },
        { "name": "Bertrand", "age": 50 },
        { "name": "Grandpa", "age": 80 }
    ] })

