const pregReplace = function (a, b, c, d) {
    void 0 === d && (d = -1);
    var e = a.substr(a.lastIndexOf(a[0]) + 1),
        f = a.substr(1, a.lastIndexOf(a[0]) - 1),
        g = RegExp(f, e),
        i = [],
        j = 0,
        k = 0,
        l = c,
        m = [];
    if (-1 === d) {
        do m = g.exec(c), null !== m && i.push(m); while (null !== m && -1 !== e.indexOf("g"))
    } else i.push(g.exec(c));
    for (j = i.length - 1; j > -1; j--) {
        for (m = b, k = i[j].length; k > -1; k--) m = m.replace("${" + k + "}", i[j][k]).replace("$" + k, i[j][k]).replace("\\" + k, i[j][k]);
        l = l.replace(i[j][0], m)
    }
    return l
  };
  
  const parser = function (str) {
    const rules = [
      // headers
      ['/(#+)(.*)/g', function (chars, item) {
        const level = chars.length
        return createTitle(level, item)
      }],
      // code fences
      ['/`{3,}(?!.*`)/g', '<pre><code>', '</pre></code>'],
      // code
      ['/(\\`)(.*?)\\1/g', function (char, item) {
        const code = item.replace(/</g, '<span><</span>')
        return `<code>${code}</code>`
      }],
      // images
      ['/\\n\\[([^\\[]+)\\]/g', function (item) {
        return createImages(item)
      }],
      // videos
      ['/\\n\\|([^\\|]+)\\|/g', function (item) {
        return createMultimedia(item)/
      }],
      // link
      ['/\\{([^\\{]+)\\}/g', function (item) {
        return createLink(item)
      }],
      // bold
      ['/(\\*)(.*?)\\1/g', '<strong>\\2</strong>'],
      // emphasis
      ['/(\\_)(.*?)\\1/g', '<em>\\2</em>'],
      // strike
      ['/(\\~)(.*?)\\1/g', '<del>\\2</del>'],
      // unordered list
      ['/\\n\\-(.*)/g', function (item) {
        return '<ul>\n<li>' + item.trim() + '</li>\n</ul>'
      }],
      // ordered list
      ['/\\n\\+(.*)/g', function (item) {
        return '<ol>\n<li>' + item.trim() + '</li>\n</ol>'
      }],
      // definition list
      ['/\\n\\?(.*)/g', function (item) {
        return createDefinitionList(item)
      }],
      // blockquote
      ['/\\n\\>(.*)/g', function (item) {
        return createQuote(item)
      }],
      // paragraphs
      ['/\\n[^\\n]+\\n/g', function (line) {
        if (codeblock) {
          line = line.trimStart()
          if (line.includes('<')) {
            line = line.replace(/</g, '<span><</span>')
            return line
          } else {
            return line
          }
        }
        line = line.trim()
        if (line[0] === '<' && !codeblock) {
          return line
        }
        return `\n<p>${line}</p>\n`
      }]
    ]
    const fixes = [
      ['/<\\/ul>\n<ul>/g', '\n'],
      ['/<\\/ol>\n<ol>/g', '\n'],
      ['/<\\/dl>\n<dl>/g', '\n'],
      ['/<\\/blockquote>\n<blockquote>/g', '\n'],
      ['/<pre><code>\n/g', '<pre><code>'],
      ['/<em><\\/em>/g', '&#95;&#95;'],
      ['/<span><</span>em><span><</span>/em>/g', '&#95;&#95;'],
      ['/ <span><</span>strong> /g', ' * '],
      ['/ <span><</span>\\/strong> /g', ' * '],
      ['/&nbsp;/g', '&nbsp;&nbsp;&nbsp;']
    ]
  
    let codeblock = false
  
    const parseLine = function (str) {
      str = `\n${str.trim()}\n`
  
      for (let i = 0, j = rules.length; i < j; i++) {
        if (typeof rules[i][1] === 'function') {
          const _flag = rules[i][0].substr(rules[i][0].lastIndexOf(rules[i][0][0]) + 1)
          const _pattern = rules[i][0].substr(1, rules[i][0].lastIndexOf(rules[i][0][0]) - 1)
          const reg = new RegExp(_pattern, _flag)
  
          const matches = [...str.matchAll(reg)]
  
          if (matches.length > 0) {
            matches.forEach(match => {
              // If more than one occurence on the same line
              if (matches.length > 1) {
                const rule = rules[i][0].slice(0, -1)
                if (match.length > 1) {
                  str = pregReplace(rule, rules[i][1](match[1], match[2]), str)
                } else {
                  str = pregReplace(rule, rules[i][1](match[0]), str)
                }
              } else {
                // If only one occurence on the same line
                if (match.length > 1) {
                  str = pregReplace(rules[i][0], rules[i][1](match[1], match[2]), str)
                } else {
                  str = pregReplace(rules[i][0], rules[i][1](match[0]), str)
                }
              }
            })
          }
        } else {
          if (str === '\n```\n' && codeblock) {
            str = rules[i][2]
            codeblock = false
          } else if (str === '\n```\n' && !codeblock) {
            str = rules[i][1]
            codeblock = true
          } else {
            str = pregReplace(rules[i][0], rules[i][1], str)
          }
        }
      }
      return str.trim()
    }
  
    str = str.split('\n')
    let rtn = []
    for (let i = 0, j = str.length; i < j; i++) {
      rtn.push(parseLine(str[i]))
    }
    rtn = rtn.join('\n')
  
    for (let i = 0, j = fixes.length; i < j; i++) {
      rtn = pregReplace(fixes[i][0], fixes[i][1], rtn)
    }
  
    return rtn
  }
  
  function extractText (text) {
    const regexuuu = /"((?:\\[\s\S]|[^\\])+?)"(?!\*)/g
    const match = text.match(regexuuu)[0].replace(/"/g, '').trim()
    return match
  }
  
  function createImages (item) {
    let el = item
    el = el.replace('[', '').replace(']', '')
    const imgArr = el.split(',')
    let imgHtml = ''
  
    if (imgArr.length === 1) {
      imgHtml = `<img loading="lazy" src="${imgArr[0]}">`
    }
  
    if (imgArr.length === 2) {
      const alt = ` alt="${imgArr[1].trim()}"`
      imgHtml = `<img loading="lazy" src="${imgArr[0].trim()}"${alt}>`
    }
  
    if (imgArr.length === 3) {
      const alt = ` alt="${imgArr[1].trim()}"`
      imgHtml = `<img loading="lazy" src="${imgArr[0]}"${alt}>`
      const caption = `<figcaption>${extractText(el)}</figcaption>`
      imgHtml = `<figure>${imgHtml}${caption}</figure>`
    }
  
    return imgHtml
  }
  
  function createLink (item) {
    let el = item
    el = el.replace('{', '').replace('}', '')
    const textLink = extractText(el)
    const linkElem = el.split(',')
    const aria = linkElem.length > 2 ? ` title="${linkElem[2].trim()}" aria-label="${linkElem[2].trim()}"` : ''
    const html = `<a href="${linkElem[0]}"${aria}>${textLink}</a>`
    return html
  }
  
  function createQuote (item) {
    let el = item
    el = el.replace(/~/g, '')
    const citation = extractText(el)
    el = el.replace(citation, '')
    el = el.split(',')
    const url = el[3] ? ` cite="${el[3].trim()}"` : ''
    const source = el[2] ? `, <cite>${el[2]}</cite>` : ''
    const author = el[1] ? `<footer>—${el[1].trim()}${source}</footer>` : ''
    const html = `<blockquote${url}><p>${citation}</p>${author}</blockquote>`
    return html
  }
  
  function createDefinitionList (item) {
    item = item.split(':')
    const term = item[0].trim()
    const definition = item[1].trim()
    const html = `<dl><dt>${term}</dt><dd>${definition}</dd></dl>`
    return html
  }
  
  function createMultimedia (item) {
    let el = item
    el = el.split(',')
    const url = el[0].trim()
    let param = el[1] ? el[1].trim() : ''
    const mediaType = url.slice(-1)
    if (param) {
      if (param === 'g') {
        param = 'autoplay="true" playsinline="true" loop="true" mute="true" preload="metadata"'
      } else {
        param = 'controls preload="metadata"'
      }
    } else {
      param = 'controls preload="metadata"'
    }
  
    let html = ''
    switch (mediaType) {
      case '4':
        html = `<video ${param} src="${url}" type="video/mp4"></video>`
        break
      case '3':
        html = `<audio ${param} src="${url}" type="audio/mpeg"></audio>`
        break
      default:
        break
    }
    return html
  }
  
  function createTitle (level, item) {
    const count = level
    const title = item.trim()
    const kebab = toKebab(title)
    const link = `<a href="#${kebab}" aria-label="${title} permalink" style="display: inline-block;width: 100%;height: 100%;position: absolute;"></a>`
    const html = `<h${count} id="${kebab}" style="position:relative;">${link}${title}</h${count}>`
    return html
  }
  
  function toKebab (text) {
    const toKebabCase = text && text
      .match(/[A-Z]{2,}(?=[A-Z][a-z]+[0-9]*|\b)|[A-Z]?[a-z]+[0-9]*|[A-Z]|[0-9]+/g)
      .map(x => x.toLowerCase())
      .join('-')
    return toKebabCase
  }
  