mustache = require("mustache");

template = "{{#people}}Hello, {{name}}.  You are {{age}} years old.\n{{/people}}";
mustache.parse(template)
for (var i = 0; i < 1000000; i++) {
    mustache.render(template, { "people": [
        { "name": "derek", "age": 20 },
        { "name": "Bertrand", "age": 50 },
        { "name": "Grandpa", "age": 80 }
    ] });
}

